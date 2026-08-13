#include "sstable_manager.hpp"

SSTableManager::SSTableManager(const std::string& directory)
    : directory_(directory), sstable_writer_(std::make_unique<SSTableWriter>(directory)) {
}

void SSTableManager::addSSTable(const std::vector<std::pair<int, MemTable::Entry>>& entries) {
    auto sst_meta = sstable_writer_->write(entries);
    std::unique_lock lock(mutex_);
    auto reader = std::make_unique<SSTableReader>(sst_meta.base);
    sstables_.push_back(SSTableHandle{std::move(sst_meta), std::move(reader)});
}

std::optional<int> SSTableManager::get(const int key) {
    std::shared_lock lock(mutex_);
    for (size_t i = sstables_.size(); i-- > 0; ) {
        if (!sstables_[i].meta.bloom_filter.mayContain(key))
            continue;   

        auto val = sstables_[i].reader->get(key);
        if (val.status == SSTableReader::getVal::FOUND) {
            return val.value;
        } else if (val.status == SSTableReader::getVal::DELETED) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

void SSTableManager::loadSSTables() {
    sstables_.clear();

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
            std::string base = path.substr(0, path.size() - 4);

            SSTableWriter::Meta meta{base, BloomFilter::deserialize(base)};
            auto reader = std::make_unique<SSTableReader>(base);
            sstables_.push_back(SSTableHandle{std::move(meta), std::move(reader)});
        }
    }
}
