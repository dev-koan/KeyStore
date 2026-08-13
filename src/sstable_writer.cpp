#include "sstable_writer.hpp"

SSTableWriter::SSTableWriter(const std::string& dir) : dir_(dir), next_id_(0) {}

std::pair<size_t, size_t> optimalBloomParams(size_t n) {
    n = std::max(n, size_t(1));
    size_t bits = std::max(n * 10, size_t(64));
    return {bits, 6};
}

SSTableWriter::Meta SSTableWriter::write(const std::vector<std::pair<int, MemTable::Entry>>& entries) {

    if (!std::filesystem::exists(dir_)) {
        std::filesystem::create_directories(dir_);
    }

    std::ostringstream ossSST;
    ossSST << dir_ << "/sstable_" << next_id_ << ".sst";
    const std::string filename = ossSST.str();

    std::ostringstream ossIndex;
    ossIndex << dir_ << "/sstable_" << next_id_ << ".idx";
    const std::string indexname = ossIndex.str();

    std::ostringstream ossBF;
    ossBF << dir_ << "/sstable_" << next_id_ << ".bf";
    const std::string bfname = ossBF.str();

    next_id_++;

    std::ofstream out(filename, std::ios::trunc | std::ios::binary);
    std::ofstream index_out(indexname, std::ios::trunc | std::ios::binary);

    if (!out.is_open() || !index_out.is_open())
        throw std::runtime_error("Failed to create SSTable files");
    
    size_t count = 0;
    auto [bit_count, hash_count] = optimalBloomParams(entries.size());
    BloomFilter bf(bit_count, hash_count);

    for (const auto& [key, entry] : entries) {
        if (count % index_window_ == 0) {
            SSTableIndexEntry index_entry{key, static_cast<uint64_t>(out.tellp())};
            index_out.write(reinterpret_cast<const char*>(&index_entry), sizeof(index_entry));
        }
        
        SSTableEntry sst_entry{key, entry.value, entry.is_tombstone ? 1 : 0};
        out.write(reinterpret_cast<const char*>(&sst_entry), sizeof(sst_entry));
        bf.add(key);
        count++;
    }
    bf.serialize(bfname);

    out.flush();
    index_out.flush();
    out.close();
    index_out.close();

    std::string base_path = dir_ + "/sstable_" + std::to_string(next_id_ - 1);

    return Meta{base_path, bf};
}
