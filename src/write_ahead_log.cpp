#include "write_ahead_log.hpp"

WriteAheadLog::WriteAheadLog(const std::string& file) : logfilename_(file) {
    logstream_ = fopen(logfilename_.c_str(), "a+b");
    if (!logstream_)
        throw std::runtime_error("Error occuerd while opening the file");
}

WriteAheadLog::~WriteAheadLog() {
    if (logstream_)
        fclose(logstream_);
}

void WriteAheadLog::appendPut(const int key, const int value) {
    std::string entry = "PUT " + std::to_string(key) + " " + std::to_string(value) + "\n";
    writeToDisk(entry);
}

void WriteAheadLog::appendDelete(const int key) {
    std::string entry = "DELETE " + std::to_string(key) + "\n";
    writeToDisk(entry); 
}

void WriteAheadLog::appendBatch(const std::vector<std::string>& entries) {
    std::string batch;

    size_t total_size = 0;
    for (auto& entry : entries)
        total_size += entry.size();
    batch.reserve(total_size);

    for (auto& entry : entries) 
        batch += entry;

    writeToDisk(batch);
}

void WriteAheadLog::replay(MemTable& map) {
    fclose(logstream_);
    std::ifstream logs(logfilename_);
    std::string line;

    while (std::getline(logs, line)) {
        if (line.empty())
            continue;

        std::istringstream iss(line);
        std::string op;
        int key, val;

        iss >> op >> key;
        if (op == "PUT") {
            iss >> val;
            map.put(key, val);
        } else if (op == "DELETE") {
            map.erase(key);
        }
    }

    logstream_ = fopen(logfilename_.c_str(), "a+b");
}

void WriteAheadLog::writeToDisk(const std::string& entry) {
    fprintf(logstream_, "%s", entry.c_str());
    fflush(logstream_);

    #ifdef _WIN32
        _commit(_fileno(logstream_));
    #else
        fsync(fileno(logstream_));
    #endif
}

void WriteAheadLog::reset() {
    fclose(logstream_);
    logstream_ = fopen(logfilename_.c_str(), "w+b");
}