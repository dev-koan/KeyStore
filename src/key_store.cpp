#include "key_store.hpp"

KeyStore::KeyStore(const std::string& logfile, const int filesize, const bool isWal) : wal_(std::make_unique<WriteAheadLog>(logfile)),
        active_(std::make_unique<MemTable>(filesize * KILOBYTE)), sstable_manager_(std::make_unique<SSTableManager>("sstables")), 
        isWal_(isWal), max_mem_(filesize * KILOBYTE) {
    sstable_manager_->loadSSTables();
    if (wal_)
        wal_->replay(*active_);
    
    flush_thread_ = std::thread(&KeyStore::flushLoop, this);
}

KeyStore::~KeyStore() {
    shutting_down_ = true;

    queue_cv_.notify_all();
    if (flush_thread_.joinable())
        flush_thread_.join();
    
    if (!active_->isEmpty())
        sstable_manager_->addSSTable(active_->toSortedEntries());
}

std::optional<int> KeyStore::getValue(const int key) {
    std::shared_ptr<MemTable> active_snapshot;
    std::vector<std::shared_ptr<MemTable>> immutable_snapshot;
    {
        std::lock_guard<std::mutex> lock(mem_mutex_);
        active_snapshot = active_;
        immutable_snapshot = immutable_;
    }

    auto val = active_snapshot->get(key);
    if (val.has_value()) {
        if (val.value().is_tombstone)
            return std::nullopt;
        return val.value().value;
    }

    // check immutable memtables, newest first (they may not be flushed yet)
    for (auto it = immutable_snapshot.rbegin(); it != immutable_snapshot.rend(); ++it) {
        auto v = (*it)->get(key);
        if (v.has_value()) {
            if (v.value().is_tombstone)
                return std::nullopt;
            return v.value().value;
        }
    }

    return sstable_manager_->get(key);
}


void KeyStore::putKey(const int key, const int val) {
    if (isWal_)
        wal_->appendPut(key, val);

    std::shared_ptr<MemTable> current;
    {
        std::lock_guard<std::mutex> lock(mem_mutex_);
        current = active_;
    }

    current->put(key, val);

    if (current->isFull()) {
        std::shared_ptr<MemTable> full_table;
        {
            std::lock_guard<std::mutex> lock(mem_mutex_);
            if (active_ == current) {   // avoid double-swap if 2 writers raced here
                full_table = active_;
                immutable_.push_back(full_table);
                active_ = std::make_shared<MemTable>(max_mem_);
            }
        }
        if (full_table)
            scheduleFlush(full_table);
    }
}

void KeyStore::deleteKey(const int key) {
    if (isWal_)
        wal_->appendDelete(key);

    std::shared_ptr<MemTable> current;
    {
        std::lock_guard<std::mutex> lock(mem_mutex_);
        current = active_;
    }
    current->erase(key);
}

bool KeyStore::hasKey(const int key) {
    return getValue(key).has_value();
}


void KeyStore::scheduleFlush(std::shared_ptr<MemTable> table) {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        flush_queue_.push(std::move(table));
    }
    queue_cv_.notify_one();
}

void KeyStore::flushLoop() {
    while (true) {
        std::shared_ptr<MemTable> table;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] {
                return !flush_queue_.empty() || shutting_down_;
            });

            if (flush_queue_.empty() && shutting_down_)
                return;

            table = std::move(flush_queue_.front());
            flush_queue_.pop();
        }

        sstable_manager_->addSSTable(table->toSortedEntries());

        {
            std::lock_guard<std::mutex> lock(mem_mutex_);
            immutable_.erase(
                std::remove(immutable_.begin(), immutable_.end(), table),
                immutable_.end());
        }
    }
}
