#include "bloom_filter.hpp"

BloomFilter::BloomFilter(size_t bits, size_t hashes)
    : bit_count_(bits),
      hash_count_(hashes),
      bits_((bits + 7) / 8, 0) {}

uint64_t BloomFilter::hash(const int key, size_t seed) const {
    return std::hash<int>{}(key + seed);
}

void BloomFilter::add(const int key) {
    for (size_t i = 0; i < hash_count_; i++) {
        uint64_t h = hash(key, i) % bit_count_;
        bits_[h / 8] |= (1 << (h % 8));
    }
}

bool BloomFilter::mayContain(const int key) const {
    for (size_t i = 0; i < hash_count_; i++) {
        uint64_t h = hash(key, i) % bit_count_;
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

    BloomFilter bf(bits, hashes);
    in.read(reinterpret_cast<char*>(bf.bits_.data()), bf.bits_.size());
    return bf;
}
