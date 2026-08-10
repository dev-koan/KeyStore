#include "key_store.hpp"

KeyStore::KeyStore(const std::string& logfile, const int filesize, const bool isWal) : wal_(std::make_unique<WriteAheadLog>(logfile)),
        active_(std::make_unique<MemTable>(filesize * KILOBYTE)), sstable_manager_(std::make_unique<SSTableManager>("sstables")), 
        isWal_(isWal), max_mem_(filesize * KILOBYTE) {
    sstable_manager_->loadSSTables();
    if (wal_)
        wal_->replay(*active_);
}

KeyStore::~KeyStore() {
    if (!active_->isEmpty())
        sstable_manager_->addSSTable(active_->toSortedEntries());
    shutting_down_ = true;
}

std::optional<int> KeyStore::getValue(const int key) {
    auto val = active_->get(key);
    if (val.has_value()){
        if (val.value().is_tombstone)
            return std::nullopt;
        return val.value().value;
    }
       
    return sstable_manager_->get(key);
}

void KeyStore::putKey(const int key, const int val) {
    if (isWal_)
        wal_->appendPut(key, val);

    active_->put(key, val);

    if (active_->isFull()) {
        sstable_manager_->addSSTable(active_->toSortedEntries());
        active_ = std::make_unique<MemTable>(max_mem_);
    }
}

void KeyStore::deleteKey(const int key) {
    if (isWal_)
        wal_->appendDelete(key);
    
    active_->erase(key);
}

bool KeyStore::hasKey(const int key) {
    return getValue(key).has_value();
}
