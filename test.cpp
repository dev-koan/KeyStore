#include <cstdlib>
#include <iostream>
#include <iostream>
#include <cassert>
#include <filesystem>
#include "key_store.hpp"

[[noreturn]] void hardCrash() {
    std::fflush(stdout);
    std::_Exit(1);  // no destructors, no flushing
}

void clean() {
    std::filesystem::remove_all("sstables");
    std::filesystem::remove("wal.log");
}

void test_wal_recovery() {
    clean();

    {
        KeyStore store("wal.log", 1, true);

        for (int i = 0; i < 30; i++)
            store.putKey(i, i * 10);

        hardCrash();
    }
}

void verify_wal_recovery() {
    KeyStore store("wal.log", 1, true);

    for (int i = 0; i < 30; i++) {
        auto v = store.getValue(i);
        assert(v.has_value());
        assert(*v == i * 10);
    }

    std::cout << "WAL recovery passed\n";
}

void test_flush_and_crash() {
    clean();

    {
        KeyStore store("wal.log", 1, true);

        for (int i = 0; i < 40; i++)
            store.putKey(i, i * 2);

        hardCrash();
    }
}

void verify_flush_and_crash() {
    KeyStore store("wal.log", 1, true);

    for (int i = 0; i < 40; i++) {
        auto v = store.getValue(i);
        assert(v.has_value());
        assert(*v == i * 2);
    }

    std::cout << "Flush + crash passed\n";
}

void test_delete_and_compaction() {
    clean();

    {
        KeyStore store("wal.log", 1, true);

        for (int i = 0; i < 100; i++)
            store.putKey(i, i);

        for (int i = 30; i < 60; i++)
            store.deleteKey(i);
    }

    {
        KeyStore store("wal.log", 1, true);

        for (int i = 0; i < 30; i++)
            assert(store.getValue(i).value() == i);

        for (int i = 30; i < 60; i++)
            assert(!store.getValue(i).has_value());

        for (int i = 60; i < 100; i++)
            assert(store.getValue(i).value() == i);
    }

    std::cout << "Deletes passed\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./test <phase>\n";
        return 1;
    }

    std::string phase = argv[1];

    if (phase == "crash1") test_wal_recovery();
    if (phase == "verify1") verify_wal_recovery();

    if (phase == "crash2") test_flush_and_crash();
    if (phase == "verify2") verify_flush_and_crash();

    if (phase == "delete") test_delete_and_compaction();

    return 0;
}