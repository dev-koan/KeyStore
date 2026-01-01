#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <memory>
#include <filesystem>
#include "mem_table.hpp"
#include "bloom_filter.hpp"
#include "sstable_reader.hpp"

class SSTableWriter {
    private:
        const std::string dir_;
        uint64_t next_id_;
        size_t index_window_ = 128;

    public:
        struct Meta {
            std::string base;
            BloomFilter bloom_filter;
        };
        struct SSTableEntry {
            int key;
            int value;
            int is_tombstone;
        };
        struct SSTableIndexEntry {
            int key;
            uint64_t offset;
        };
        explicit SSTableWriter(const std::string&);
        ~SSTableWriter() = default;

        Meta write(const std::vector<std::pair<int, MemTable::Entry>>&);
};
