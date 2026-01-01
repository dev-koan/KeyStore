#pragma once
    
#include <string>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <io.h>
#include "mem_table.hpp"

class WriteAheadLog {
    private:
        std::string logfilename_;
        FILE* logstream_;
        void writeToDisk(const std::string&);

    public:
        explicit WriteAheadLog(const std::string&);
        ~WriteAheadLog();

        void appendPut(const int, const int);
        void appendDelete(const int);
        void replay(MemTable&);
        void appendBatch(const std::vector<std::string>&);
        void reset();
};
