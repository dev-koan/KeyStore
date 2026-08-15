# KeyStore

### Phase 1
- Implement Write Ahead Logging with a Persistance Manager using in memory data structure `unordered_map`

### Phase 2
- Add Memtable class which uses `map` upto a certain number of entries
- Add SSTableWriter class that takes the entries from the memtable and writes them in a SSTable txt file
- Add SSTableReader class which searches for the latest value for a key
- Manages reads and writes and keeps track of the sstables created
- Add Bloomfilters to prevent reading from SSTable files where the required key may not exist.
- Add key indexing with file offsets to binary search for the key in a file

### Phase 3
- Add a background thread to write SSTable files

### Phase 4
- Use memory mapped files

### Phase 5
- Write Ahead Logging may require some fixes when multi-threading
- Better benchmarking and testing infrastructure

### Phase 6 (Not Implemented yet) - Support 1 billion+ read/write ops sub 50 seconds
- Add compaction
- Multi threads for writing 
- Support concurrent reads and write while ensuring eventual consistency 

### Build with CMake (Linux)

Configure:
```powershell
cmake -S . -B build
```

Build:
```powershell
cmake --build build
```

Run functional tests:
```powershell
./build/runFunc.exe crash1
./build/runFunc.exe verify1
./build/runFunc.exe crash2
./build/runFunc.exe verify2
./build/runFunc.exe delete
```

Run benchmark:
```powershell
./build/runB.exe
```

### Benchmarking Results (10 Million Operation)

```
Write Results
Operations: 1000000
Duration: 0.3478130800 seconds
Throughput: 2875107.5146455104 ops/second
Average Latency: 0.0000002881 seconds
P50 Latency: 0.0000002380 seconds
P99 Latency: 0.0000007730 seconds
P999 Latency: 0.0000069080 seconds
Min Latency: 0.0000002070 seconds
Max Latency: 0.0013862800 seconds

Read Results
Operations: 1000000
Duration: 3.6723452300 seconds
Throughput: 272305.5533643279 ops/second
Average Latency: 0.0000036013 seconds
P50 Latency: 0.0000033290 seconds
P99 Latency: 0.0000100120 seconds
P999 Latency: 0.0000419400 seconds
Min Latency: 0.0000002500 seconds
Max Latency: 0.0007654720 seconds

Mixed Workload (70% read, 30% write)

Mixed Read Results
Operations: 700069
Duration: 1.9742083520 seconds
Throughput: 354607.4553330630 ops/second
Average Latency: 0.0000025040 seconds
P50 Latency: 0.0000023630 seconds
P99 Latency: 0.0000050010 seconds
P999 Latency: 0.0000264260 seconds
Min Latency: 0.0000002110 seconds
Max Latency: 0.0026154640 seconds

Mixed Write Results
Operations: 299931
Duration: 1.9742083520 seconds
Throughput: 151924.6941165813 ops/second
Average Latency: 0.0000003753 seconds
P50 Latency: 0.0000003130 seconds
P99 Latency: 0.0000007750 seconds
P999 Latency: 0.0000039670 seconds
Min Latency: 0.0000002050 seconds
Max Latency: 0.0003951630 seconds
All benchmarks completed
```
