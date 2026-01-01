#include <flush_buffer.hpp>

FlushBuffer::FlushBuffer(const size_t maxsize) : maxbuffersize_(maxsize) {
}

void FlushBuffer::add(const std::string& entry) {
    buffer_.push_back(entry);
}

bool FlushBuffer::isFull() const {
    return buffer_.size() >= maxbuffersize_;
}

std::vector<std::string> FlushBuffer::drain() {
    std::vector<std::string> temp;
    std::swap(buffer_, temp);

    return temp;
}

void FlushBuffer::clear() {
    buffer_.clear();
}