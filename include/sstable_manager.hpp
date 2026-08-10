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

    void addSSTable(const std::vector<std::pair<int, MemTable::Entry>>& entries);
    std::optional<int> get(const int);
    void loadSSTables();
    
private:
    std::string directory_;
    std::vector<SSTableWriter::Meta> sstable_files_;
    std::unique_ptr<SSTableWriter> sstable_writer_;
    std::mutex mutex_;
};
