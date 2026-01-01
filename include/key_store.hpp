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
#include "sstable_write.hpp"
#include "sstable_manager.hpp"

class KeyStore {
    private:
        std::atomic<std::shared_ptr<MemTable>> active_;
        std::atomic<std::shared_ptr<MemTable>> frozen_;
        std::unique_ptr<WriteAheadLog> wal_;
        std::unique_ptr<SSTableManager> sstable_manager_;
        const bool isWal_; 
        const int max_mem_;

        std::mutex flush_mutex_;
        std::condition_variable flush_cv_;
        std::condition_variable frozen_cv_;
        std::thread flush_thread_;
        bool shutting_down_ = false;

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

