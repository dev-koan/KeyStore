#pragma once

#include <vector>
#include <string>

class FlushBuffer {
    private:
        size_t maxbuffersize_;
        std::vector<std::string> buffer_;

    public:
        explicit FlushBuffer(const size_t);
        ~FlushBuffer() = default; 

        void add(const std::string&);
        bool isFull() const;
        std::vector<std::string> drain();
        void clear();
};
