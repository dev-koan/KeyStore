#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <filesystem>
#include <mutex>
#include "sstable_reader.hpp"
#include "bloom_filter.hpp"
#include "mem_table.hpp"
#include "sstable_writer.hpp"

class SSTableManager {
public:
    explicit SSTableManager(const std::string&);

    struct SSTableHandle {
        SSTableWriter::Meta meta;
        std::unique_ptr<SSTableReader> reader;
    };
    

    void addSSTable(const std::vector<std::pair<int, MemTable::Entry>>& entries);
    std::optional<int> get(const int);
    void loadSSTables();

private:
    std::string directory_;
    std::unique_ptr<SSTableWriter> sstable_writer_;
    std::vector<SSTableHandle> sstables_;
    mutable std::shared_mutex mutex_;
};
