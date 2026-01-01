#include "key_store.hpp"

KeyStore::KeyStore(const std::string& logfile, const int filesize, const bool isWal) : wal_(std::make_unique<WriteAheadLog>(logfile)),
        active_(std::make_shared<MemTable>(filesize * 1024)), sstable_manager_(std::make_unique<SSTableManager>("sstables")), 
        isWal_(isWal), max_mem_(filesize * 1024) {
    sstable_manager_->loadSSTables();
    if (wal_)
        wal_->replay(*active_.load());
    flush_thread_ = std::thread([this]() {
        while (true) {
            {
                std::unique_lock<std::mutex> lk(flush_mutex_);
                flush_cv_.wait(lk, [&]{
                    return frozen_.load() || shutting_down_;
                });
                
                if (shutting_down_ && !frozen_.load())
                    break;
                
            }

            sstable_manager_->addSSTable(frozen_.load()->toSortedEntries());
            frozen_.store(NULL);

            frozen_cv_.notify_one();

            if (isWal_)
                wal_->reset();
        }
    });
}

KeyStore::~KeyStore() {
    {
        std::lock_guard<std::mutex> lk(flush_mutex_);
        shutting_down_ = true;
    }
    flush_cv_.notify_one();
    flush_thread_.join();
}

std::optional<int> KeyStore::getValue(const int key) {
    auto val = active_.load()->get(key);
    if (val.has_value()){
        if (val.value().is_tombstone)
            return std::nullopt;
        return val.value().value;
    }

    if (frozen_.load()) {
        val = frozen_.load()->get(key);
        if (val.has_value()){
            if (val.value().is_tombstone)
                return std::nullopt;
            return val.value().value;
        }
    }
        
    return sstable_manager_->get(key);

}

void KeyStore::putKey(const int key, const int val) {
    if (isWal_)
        wal_->appendPut(key, val);

    active_.load()->put(key, val);

    if (active_.load()->isFull()) {

        std::unique_lock<std::mutex> lk(flush_mutex_);

        frozen_cv_.wait(lk, [&] {
            return frozen_.load() == NULL;
        });

        frozen_.store(active_.load());
        active_.store(std::make_shared<MemTable>(max_mem_));
        flush_cv_.notify_one();

    }
}

void KeyStore::deleteKey(const int key) {
    if (!hasKey(key))
        return;

    if (isWal_)
        wal_->appendDelete(key);
    
    active_.load()->erase(key);
}

bool KeyStore::hasKey(const int key) {
    return getValue(key).has_value();
}
