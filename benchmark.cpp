#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <random>
#include <cassert>
#include <iomanip>
#include "key_store.hpp"

const int OPS_TO_FILE = 5000;

class BenchmarkStats {
public:
    std::vector<double> latencies;
    
    void addLatency(double latency_us) {
        latencies.push_back(latency_us);
    }
    
    double getAverage() const {
        if (latencies.empty()) return 0.0;
        double sum = 0.0;
        for (double lat : latencies) sum += lat;

        return sum / latencies.size();
    }
    
    double getPercentile(double percentile) const {
        if (latencies.empty()) return 0.0;

        std::vector<double> sorted = latencies;
        std::sort(sorted.begin(), sorted.end());

        size_t index = static_cast<size_t>(percentile / 100.0 * sorted.size());
        if (index >= sorted.size()) index = sorted.size() - 1;
        return sorted[index];
    }
    
    double getThroughput(double time) const {
        if (time == 0.0) return 0.0;
        return latencies.size() / time;
    }
};
void clean() {
    #ifdef _WIN32
        std::system("rmdir /s /q sstables >nul 2>&1");
        std::system("del /f wal.log >nul 2>&1");
    #else
        std::system("rm -rf sstables 2>/dev/null");
        std::system("rm -f wal.log 2>/dev/null");
    #endif
}

void printBenchmarkResults(const std::string& test_name, 
                          const BenchmarkStats& stats,
                          double time) {
    std::cout << "\n" << test_name << " Results" << std::endl;
    std::cout << std::fixed << std::setprecision(10);
    std::cout << "Operations: " << stats.latencies.size() << std::endl;
    std::cout << "Duration: " << time << " seconds" << std::endl;
    std::cout << "Throughput: " << stats.getThroughput(time) << " ops/second" << std::endl;
    std::cout << "Average Latency: " << stats.getAverage() << " seconds" << std::endl;
    std::cout << "P50 Latency: " << stats.getPercentile(50) << " seconds" << std::endl;
    std::cout << "P99 Latency: " << stats.getPercentile(99) << " seconds" << std::endl;
    std::cout << "P999 Latency: " << stats.getPercentile(99.9) << " seconds" << std::endl;
    std::cout << "Min Latency: " << stats.getPercentile(0) << " seconds" << std::endl;
    std::cout << "Max Latency: " << stats.getPercentile(100) << " seconds" << std::endl;
}


// Benchmark write operations
void benchmarkWrites(KeyStore& store, int num_operations) {
    BenchmarkStats stats;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1000000);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_operations; i++) {
        int key = i;
        int value = dis(gen);
        
        auto op_start = std::chrono::high_resolution_clock::now();
        store.putKey(key, value);
        auto op_end = std::chrono::high_resolution_clock::now();
        
        double latency = std::chrono::duration_cast<std::chrono::duration<double>>(op_end - op_start).count();
        stats.addLatency(latency);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    
    printBenchmarkResults("Write", stats, duration);
}

// Benchmark read operations
void benchmarkReads(KeyStore& store, int num_operations) {
    
    for (int i = 0; i < num_operations; i++) {
        store.putKey(i, i * 2);
    }
    
    BenchmarkStats stats;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, num_operations - 1);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_operations; i++) {
        int key = dis(gen);
        
        auto op_start = std::chrono::high_resolution_clock::now();
        std::optional<int> value = store.getValue(key);
        auto op_end = std::chrono::high_resolution_clock::now();
        if (!value.has_value() || value.value() != key * 2) {
            std::cout << key << std::endl;
            throw std::runtime_error("Data mismatch or key not found during read benchmark");
        }
        
        double latency = std::chrono::duration_cast<std::chrono::duration<double>>(op_end - op_start).count();
        stats.addLatency(latency);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    
    printBenchmarkResults("Read", stats, duration);
}

// Benchmark mixed workload
void benchmarkMixedWorkload(KeyStore& store, int num_operations) {
    
    for (int i = 0; i < num_operations / 2; i++) {
        store.putKey(i, i * 2);
    }
    
    BenchmarkStats read_stats, write_stats;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> key_dis_write(0, num_operations - 1);
    std::uniform_int_distribution<> key_dis_read(0, (num_operations / 2) - 1);
    std::uniform_int_distribution<> op_dis(0, 99);
    std::uniform_int_distribution<> val_dis(0, 1000000);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < num_operations; i++) {
        std::string key;
        bool is_read = op_dis(gen) < 70;
        
        if (is_read) {
            int key = key_dis_read(gen);
            auto op_start = std::chrono::high_resolution_clock::now();
            std::optional<int> value = store.getValue(key);
            auto op_end = std::chrono::high_resolution_clock::now();
            double latency = std::chrono::duration_cast<std::chrono::duration<double>>(op_end - op_start).count();
            read_stats.addLatency(latency);
        } else {
            int key = key_dis_write(gen);
            auto op_start = std::chrono::high_resolution_clock::now();
            store.putKey(key, val_dis(gen));
            auto op_end = std::chrono::high_resolution_clock::now();
            double latency = std::chrono::duration_cast<std::chrono::duration<double>>(op_end - op_start).count();
            write_stats.addLatency(latency);
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
    
    std::cout << "\nMixed Workload (70% read, 30% write)" << std::endl;
    printBenchmarkResults("Mixed Read", read_stats, duration);
    printBenchmarkResults("Mixed Write", write_stats, duration);
}


int main() {
    int num_operations;
    std::cout << "KeyValue Store Test Suite" << std::endl;
    std::cout << "Enter the number of operations for each benchmark (e.g., 1000000): ";
    std::cin >> num_operations;

    const int filesize = num_operations / OPS_TO_FILE; 
    try {
        
        // Run benchmarks
        clean();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        {
            KeyStore storeBench("wal.log", filesize, false);
            benchmarkWrites(storeBench, num_operations);
        }
        clean();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        {
            KeyStore storeBench("wal.log", filesize, false);
            benchmarkReads(storeBench, num_operations);
        }
        clean();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        {
            KeyStore storeBench("wal.log", filesize, false);
            benchmarkMixedWorkload(storeBench, num_operations);
        }
        
        std::cout << "All benchmarks completed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
