#include "mem_table.hpp"

MemTable::MemTable(size_t max_size) : current_size_(0), max_size_(max_size) {};

void MemTable::put(int key, const int value) {
    table_[key].value = value;
    table_[key].is_tombstone = false;

    current_size_ += sizeof(int) + sizeof(int) + sizeof(bool) + 32;
}

void MemTable::erase(const int key) {
    if (table_.count(key)){
        table_[key].is_tombstone = true;
    } else {
        table_[key].value = -1;
        table_[key].is_tombstone = true;
        current_size_ += sizeof(int) + sizeof(int) + sizeof(bool) + 32;
    }
}

std::optional<MemTable::Entry> MemTable::get(const int key) const {
    auto it = table_.find(key);

    return it != table_.end() ? std::optional<MemTable::Entry>(it->second) : std::nullopt;
}

bool MemTable::isFull() const {
    return current_size_ >= max_size_;
}

std::vector<std::pair<int, MemTable::Entry>> MemTable::toSortedEntries() const {
    std::vector<std::pair<int, Entry>> entries;
    
    for (const auto& kv : table_)
        entries.push_back(kv);
    
    return entries;
}

void MemTable::clear() {
    table_.clear();
    current_size_ = 0;
}

