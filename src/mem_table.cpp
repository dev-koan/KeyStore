#include "mem_table.hpp"

MemTable::MemTable(size_t max_size) : current_size_(0), max_size_(max_size) {
    table_.reserve(max_size_ / (sizeof(int) + sizeof(int) + sizeof(bool) + 32));
};

void MemTable::put(int key, const int value) {
    std::unique_lock lock(mutex_);
    if (!table_.count(key))
        current_size_ += sizeof(int) + sizeof(int) + sizeof(bool) + 32;
    table_[key].value = value;
    table_[key].is_tombstone = false;
}

void MemTable::erase(const int key) {
    std::unique_lock lock(mutex_);
    if (table_.count(key)){
        table_[key].is_tombstone = true;
    } else {
        table_[key].value = -1;
        table_[key].is_tombstone = true;
        current_size_ += sizeof(int) + sizeof(int) + sizeof(bool) + 32;
    }
}

std::optional<MemTable::Entry> MemTable::get(const int key) const {
    std::shared_lock lock(mutex_);
    auto it = table_.find(key);
    return it != table_.end() ? std::optional<MemTable::Entry>(it->second) : std::nullopt;
}

bool MemTable::isFull() const {
    std::shared_lock lock(mutex_);
    return current_size_ >= max_size_;
}

std::vector<std::pair<int, MemTable::Entry>> MemTable::toSortedEntries() const {
    std::vector<std::pair<int, Entry>> entries(table_.begin(), table_.end());
    std::sort(entries.begin(), entries.end(), [](const std::pair<int, Entry>& a, const std::pair<int, Entry>& b) {
        return a.first < b.first;
    });
    return entries;
}

void MemTable::clear() {
    table_.clear();
    current_size_ = 0;
}

bool MemTable::isEmpty() const {
    return table_.empty();
}
