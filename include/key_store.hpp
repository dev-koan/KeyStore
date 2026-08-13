#pragma once

#include <iostream>
#include <unordered_map>
#include <string>
#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "persistance_manager.hpp"
#include "write_ahead_log.hpp"
#include "flush_buffer.hpp"
#include "mem_table.hpp"
#include "sstable_writer.hpp"
#include "sstable_manager.hpp"

class KeyStore {
    private:
        const int KILOBYTE = 1024;

        std::shared_ptr<MemTable> active_;
        std::vector<std::shared_ptr<MemTable>> immutable_;
        std::mutex mem_mutex_; 

        std::unique_ptr<WriteAheadLog> wal_;
        std::unique_ptr<SSTableManager> sstable_manager_;
        const bool isWal_; 
        const int max_mem_;

        std::queue<std::shared_ptr<MemTable>> flush_queue_;
        std::mutex queue_mutex_;
        std::condition_variable queue_cv_;
        std::thread flush_thread_;
        std::atomic<bool> shutting_down_{false};

        void flushLoop();
        void scheduleFlush(std::shared_ptr<MemTable> table);

    public:
        explicit KeyStore(const std::string&, const int, const bool);
        ~KeyStore();

        // Prevent copy constructor and using '='
        KeyStore(const KeyStore&) = delete;
        KeyStore& operator=(const KeyStore&) = delete;

        // Allowing moving when a temporary object wants to become permanent
        KeyStore(KeyStore&&) = delete;
        KeyStore& operator=(KeyStore&&) = delete;

        std::optional<int> getValue(const int);
        void putKey(const int, const int);
        void deleteKey(const int);
        bool hasKey(const int);
};
