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

### Phase 5 (Not Implemented yet) - Support 1 billion+ read/write ops sub 50 seconds
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

### Benchmarking Results (1 Million Operation)

```
Write Results
Operations: 1000000
Duration: 0.5128556830 seconds
Throughput: 1949866.2745636380 ops/second
Average Latency: 0.0000004395 seconds
P50 Latency: 0.0000002020 seconds
P99 Latency: 0.0000003220 seconds
P999 Latency: 0.0000005640 seconds
Min Latency: 0.0000001800 seconds
Max Latency: 0.0025220140 seconds

Read Results
Operations: 1000000
Duration: 4.0372825350 seconds
Throughput: 247691.3595545524 ops/second
Average Latency: 0.0000039571 seconds
P50 Latency: 0.0000037970 seconds
P99 Latency: 0.0000083400 seconds
P999 Latency: 0.0000267820 seconds
Min Latency: 0.0000001820 seconds
Max Latency: 0.0024382040 seconds

Mixed Workload (70% read, 30% write)

Mixed Read Results
Operations: 699358
Duration: 2.3572484520 seconds
Throughput: 296684.0425355384 ops/second
Average Latency: 0.0000028914 seconds
P50 Latency: 0.0000027880 seconds
P99 Latency: 0.0000059800 seconds
P999 Latency: 0.0000190290 seconds
Min Latency: 0.0000001390 seconds
Max Latency: 0.0023466490 seconds

Mixed Write Results
Operations: 300642
Duration: 2.3572484520 seconds
Throughput: 127539.3774232501 ops/second
Average Latency: 0.0000006795 seconds
P50 Latency: 0.0000002730 seconds
P99 Latency: 0.0000006160 seconds
P999 Latency: 0.0000046690 seconds
Min Latency: 0.0000001760 seconds
Max Latency: 0.0074823970 seconds
All benchmarks completed!
```
