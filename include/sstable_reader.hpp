#pragma once

#include <string>
#include <vector>
#include <optional>
#include <fstream>
#include <sstream>
#include <cstdint>
#include "sstable_reader.hpp"
#include "sstable_writer.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <cstring>

class SSTableReader {
public:
    enum class getVal {
        FOUND,
        NOT_FOUND,
        DELETED
    };
    struct getStatus {
        getVal status;
        std::optional<int> value;
    };
    struct Entry {
        int key;
        uint64_t offset;
    };
    explicit SSTableReader(const std::string&);
    ~SSTableReader();

    getStatus get(const int);
    uint64_t findStart(const int) const;
private:
    std::string filename_;
    std::vector<Entry> index_;
    // std::ifstream data_;
    void *data_ = NULL;
    size_t data_size_ = 0;
};
