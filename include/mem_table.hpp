#pragma once

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <optional>
#include <algorithm>

class MemTable {
public:
    explicit MemTable(size_t max_size);

    struct Entry {
        int value;
        bool is_tombstone;
    };
    void put(int key, int value);
    void erase(const int key);
    std::optional<MemTable::Entry> get(const int key) const;
    bool isFull() const;
    std::vector<std::pair<int, Entry>> toSortedEntries() const;
    void clear();
    
private:
    std::map<int, Entry> table_;
    size_t current_size_;
    size_t max_size_;
};
