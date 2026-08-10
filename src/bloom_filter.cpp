#include "bloom_filter.hpp"

BloomFilter::BloomFilter(size_t bits, size_t hashes)
    : bit_count_(bits),
      hash_count_(hashes),
      bits_((bits + 7) / 8, 0) {}

inline uint64_t mix64(uint64_t x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    return x;
}

void BloomFilter::add(const int key) {
    uint64_t h1 = mix64(key);
    uint64_t h2 = mix64(key ^ 0x9E3779B97F4A7C15ULL) | 1;

    for (size_t i = 0; i < hash_count_; i++) {
        uint64_t h = (h1 + i * h2) % bit_count_;
        bits_[h / 8] |= (1 << (h % 8));
    }
}

bool BloomFilter::mayContain(const int key) const {
    uint64_t h1 = mix64(key);
    uint64_t h2 = mix64(key ^ 0x9E3779B97F4A7C15ULL) | 1;

    for (size_t i = 0; i < hash_count_; i++) {
        uint64_t h = (h1 + i * h2) % bit_count_;
        if (!(bits_[h / 8] & (1 << (h % 8))))
            return false;
    }

    return true;
}

void BloomFilter::serialize(const std::string& path) const {
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(&bit_count_), sizeof(bit_count_));
    out.write(reinterpret_cast<const char*>(&hash_count_), sizeof(hash_count_));
    out.write(reinterpret_cast<const char*>(bits_.data()), bits_.size());
}

BloomFilter BloomFilter::deserialize(const std::string& path) {
    std::ifstream in(path + ".bf", std::ios::binary);
    size_t bits, hashes;
    in.read(reinterpret_cast<char*>(&bits), sizeof(bits));
    in.read(reinterpret_cast<char*>(&hashes), sizeof(hashes));

    if (!in.is_open())
        throw std::runtime_error("Failed to read Bloom filter from file: " + path);

    BloomFilter bf(bits, hashes);
    in.read(reinterpret_cast<char*>(bf.bits_.data()), bf.bits_.size());
    return bf;
}
