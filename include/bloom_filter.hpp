#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>

class BloomFilter {
    private:
    size_t bit_count_;
    size_t hash_count_;
    std::vector<uint8_t> bits_;
public:
    BloomFilter() = default;
    BloomFilter(size_t bits, size_t hashes);

    uint64_t hash(const int key, size_t seed) const;
    void add(const int key);
    bool mayContain(const int key) const;
    void serialize(const std::string& path) const;
    static BloomFilter deserialize(const std::string& path);
};
