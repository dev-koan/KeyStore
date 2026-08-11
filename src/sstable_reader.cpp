#include "sstable_reader.hpp"

SSTableReader::SSTableReader(const std::string& filename) : filename_(filename) {
    int fd = open((filename + ".sst").c_str(), O_RDONLY);
    if (fd != -1) {
        struct stat st;
        if (fstat(fd, &st) == -1) {
            close(fd);
            throw std::runtime_error("fstat failed for " + filename + ".sst");
        }

        data_size_ = static_cast<size_t>(st.st_size);

        if (data_size_ > 0) {
            data_ = mmap(nullptr, data_size_, PROT_READ, MAP_PRIVATE, fd, 0);
            if (data_ == MAP_FAILED) {
                data_ = nullptr;
                close(fd);
                throw std::runtime_error("mmap failed for " + filename + ".sst");
            }
            madvise(data_, data_size_, MADV_RANDOM);
        }

        close(fd);
    }

    std::ifstream index_file(filename + ".idx", std::ios::binary);
    if (!index_file.is_open())
        return;

    int key;
    uint64_t offset;
    SSTableWriter::SSTableIndexEntry index_entry;
    while (index_file.read(reinterpret_cast<char*>(&index_entry), sizeof(index_entry)))
        index_.push_back({index_entry.key, index_entry.offset});
    index_file.close();
}

SSTableReader::~SSTableReader() {
    if (data_)
        munmap(data_, data_size_);
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
    if (!data_)
        return {SSTableReader::getVal::NOT_FOUND, std::nullopt};
        
    const char* base = static_cast<const char*>(data_);
    uint64_t pos = findStart(key);

    while (pos + sizeof(SSTableWriter::SSTableEntry) <= data_size_) {
        SSTableWriter::SSTableEntry sst_entry;
        std::memcpy(&sst_entry, base + pos, sizeof(sst_entry));
        
        if (sst_entry.key == key) {
            if (sst_entry.is_tombstone == 1)
                return {SSTableReader::getVal::DELETED, std::nullopt};
            return {SSTableReader::getVal::FOUND, sst_entry.value};
        }
        if (sst_entry.key > key)
            break;
         
        pos += sizeof(sst_entry);
    }

    return {SSTableReader::getVal::NOT_FOUND, std::nullopt};   
}
