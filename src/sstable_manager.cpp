#include "sstable_manager.hpp"


SSTableManager::SSTableManager(const std::string& directory)
    : directory_(directory), sstable_writer_(std::make_unique<SSTableWriter>(directory)) {
}

void SSTableManager::addSSTable(const std::vector<std::pair<int, MemTable::Entry>>& entries) {
    auto sst_meta = sstable_writer_->write(entries);
    sstable_files_.push_back(std::move(sst_meta));
}

std::optional<int> SSTableManager::get(const int key) {
    for (size_t i = sstable_files_.size() - 1; i >= 0; i--) {
        if (!sstable_files_[i].bloom_filter.mayContain(key))
            continue;   

        SSTableReader reader(sstable_files_[i].base);
        auto val = reader.get(key);
        if (val.status == SSTableReader::getVal::FOUND) {
            return val.value;
        } else if (val.status == SSTableReader::getVal::DELETED) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

void SSTableManager::loadSSTables() {
    sstable_files_.clear();

    if (!std::filesystem::exists(directory_))
        return;
    
    std::vector<std::filesystem::directory_entry> entries;
    for (const auto& entry : std::filesystem::directory_iterator(directory_)) {
        if (entry.path().extension() == ".sst") {
            entries.push_back(entry);
        }
    }
    std::sort(entries.begin(), entries.end(), [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b) {
        size_t a_id = std::stoull(a.path().stem().string());
        size_t b_id = std::stoull(b.path().stem().string());
        return a_id < b_id;
    });

    for (const auto& entry : entries) {
        if (entry.path().extension() == ".sst") {
            std::string path = entry.path().string();
            sstable_files_.push_back(SSTableWriter::Meta{path.substr(0, path.size() - 4), BloomFilter::deserialize(path.substr(0, path.size() - 4))});
        }
    }
}

