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

### Phase 4 (In Progress)
- Use memory mapped files and add caching

### Benchmarking Results (Until Phase 3)
<img width="847" height="1224" alt="image" src="https://github.com/user-attachments/assets/1c6a6734-14f5-4e07-8116-942ad7808801" />
