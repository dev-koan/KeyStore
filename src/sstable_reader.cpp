#include "sstable_reader.hpp"

SSTableReader::SSTableReader(const std::string& filename) : filename_(filename), data_(filename + ".sst",  std::ios::binary) {
    std::ifstream index_file(filename + ".idx", std::ios::binary);
    if (!index_file.is_open())
        return;

    int key;
    uint64_t offset;
    SSTableWriter::SSTableIndexEntry index_entry;
    while (index_file.read(reinterpret_cast<char*>(&index_entry), sizeof(index_entry)))
        index_.push_back({index_entry.key, index_entry.offset});
    
}

uint64_t SSTableReader::findStart(const int key) const {
    if (index_.empty())
        return 0;

    size_t left = 0, right = index_.size();

    while (left + 1 < right) {
        size_t mid = (right + left) / 2;

        if (index_[mid].key <= key)
            left = mid;
         else
            right = mid;
        
    }

    if (index_[left].key > key)
        return 0;

    return index_[left].offset;
    
}

SSTableReader::getStatus SSTableReader::get(const int key) {
    if (!data_.is_open())
        return {SSTableReader::getVal::NOT_FOUND, std::nullopt};
        
    data_.clear();
    data_.seekg(findStart(key), std::ios::beg);

    SSTableWriter::SSTableEntry sst_entry;
    while (data_.read(reinterpret_cast<char*>(&sst_entry), sizeof(sst_entry))) {
        if (sst_entry.key == key) {
            if (sst_entry.is_tombstone == 1)
                return {SSTableReader::getVal::DELETED, std::nullopt};
            return {SSTableReader::getVal::FOUND, sst_entry.value};
        }
        if (sst_entry.key > key)
            break;
    }

    return {SSTableReader::getVal::NOT_FOUND, std::nullopt};   
}

