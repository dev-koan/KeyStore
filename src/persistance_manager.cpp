// #include "persistance_manager.hpp"

// PersistanceManager::PersistanceManager(const std::string& file) : filename_(file) {
// }

// void PersistanceManager::save(const MemTable& data) {
//     std::ofstream data_file(filename_);

//     if (!data_file.is_open()){
//         throw std::runtime_error("Cannot open data file");
//     } 

//     for (const auto& pair : data.toSortedEntries())
//         data_file << pair.first << "=" << pair.second.value << std::endl;

//     data_file.close();
// }

// void PersistanceManager::load(MemTable& data) {
//     std::ifstream data_file(filename_);
//     std::string line;

//     if (!data_file.is_open()){
//         throw std::runtime_error("Cannot open data file");
//     }

//     while (std::getline(data_file, line)) {
//         if (line.empty())
//             continue;

//         const size_t delimeter_pos = line.find('=');
//         if (delimeter_pos == std::string::npos) {
//             throw std::runtime_error("Invalid data format");
//         }
//         try {
//             const std::string key = line.substr(0, delimeter_pos);
//             const int value = std::stoi(line.substr(delimeter_pos + 1));
//             data.put(key, value);
//         } catch (...) {
//             throw std::runtime_error("Invalid data format");
//         }
//     }
// }