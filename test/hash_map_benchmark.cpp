// Copyright (c) 2021 Tencent, Inc.
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#include <cstdlib>
#include <string>
#include <unordered_map>

#include "folly/concurrency/ConcurrentHashMap.h"
#include "folly/AtomicHashMap.h"
#include "folly/AtomicUnorderedMap.h"
#include "tbb/concurrent_hash_map.h"
#include "tbb/concurrent_unordered_map.h"
#include "benchmark/benchmark.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/node_hash_map.h"

using namespace std;

string RandomStr()
{
    static const char* kAlphabets = "abcdefghijklmnoqprstuvwxyzABCDEFGHIJKLMNQOPQRSTUVWXYZ0123456789";
    static int kLen = strlen(kAlphabets);

    string result;
    for (int i = 0; i < 10; ++i) {
        char c = kAlphabets[rand() % kLen];
        result += c;
    }
    return result;
}

static void BM_FollyConcurrentHashMapInsert(benchmark::State& state)
{
    static folly::ConcurrentHashMap<std::string, std::string> map;
    for (auto _ : state) {
        benchmark::DoNotOptimize(map.insert(RandomStr(), RandomStr()));
        benchmark::ClobberMemory();
    }
}
    
BENCHMARK(BM_FollyConcurrentHashMapInsert)->ThreadRange(1, 32);

static void BM_FollyConcurrentHashMapInsertAndFind(benchmark::State& state)
{
    static folly::ConcurrentHashMap<std::string, std::string> map;
    for (auto _ : state) {
        auto key = RandomStr();
        benchmark::DoNotOptimize(map.insert(key, RandomStr()));
        benchmark::DoNotOptimize(map.find(key));
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_FollyConcurrentHashMapInsertAndFind)->ThreadRange(1, 32);

static void BM_FollyConcurrentHashMapInsertFindAndErase(benchmark::State& state)
{
    static folly::ConcurrentHashMap<std::string, std::string> map;
    for (auto _ : state) {
        auto key = RandomStr();
        map.insert(key, RandomStr());
        benchmark::DoNotOptimize(map.find(key));
        if (rand() % 100 == 0) {
            map.erase(key);
        }
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_FollyConcurrentHashMapInsertFindAndErase)->ThreadRange(1, 32);

static void BM_FollyAtomicHashMapInsert(benchmark::State& state)
{
    static folly::AtomicHashMap<int, string> map(10000000);
    for (auto _ : state) {
        map.insert(rand(), RandomStr());
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_FollyAtomicHashMapInsert)->ThreadRange(1, 32);

static void BM_FollyAtomicHashMapInsertAndFind(benchmark::State& state)
{
    static folly::AtomicHashMap<int, string> map(10000000);
    for (auto _ : state) {
        auto key = rand();
        map.insert(key, RandomStr());
        benchmark::DoNotOptimize(map.find(key));
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_FollyAtomicHashMapInsertAndFind)->ThreadRange(1, 32);

static void BM_FollyAtomicHashMapInsertFindAndErase(benchmark::State& state)
{
    static folly::AtomicHashMap<int, string> map(10000000);
    for (auto _ : state) {
        auto key = rand();
        map.insert(key, RandomStr());
        benchmark::DoNotOptimize(map.find(key));
        if (rand() % 100 == 0) {
            map.erase(key);
        }
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_FollyAtomicHashMapInsertFindAndErase)->ThreadRange(1, 32);

static void BM_FollyAtomicUnorderedInsertMapInsert(benchmark::State& state)
{
    static folly::AtomicUnorderedInsertMap<string, string> map(10000000);
    for (auto _ : state) {
        benchmark::DoNotOptimize(map.findOrConstruct(RandomStr(),
                                                     [] (void* p) {
                                                         new (p) std::string(RandomStr());
                                                     }));
        benchmark::ClobberMemory();
    }
}
                                   
BENCHMARK(BM_FollyAtomicUnorderedInsertMapInsert)->ThreadRange(1, 32);

static void BM_FollyAtomicUnorderedInsertMapInsertAndFind(benchmark::State& state)
{
    static folly::AtomicUnorderedInsertMap<string, string> map(10000000);
    for (auto _ : state) {
        auto key = RandomStr();
        benchmark::DoNotOptimize(map.findOrConstruct(key, [=] (void* p) {
                                                              new (p) std::string(RandomStr());
                                                          }));
        benchmark::DoNotOptimize(map.find(key));
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_FollyAtomicUnorderedInsertMapInsertAndFind)->ThreadRange(1, 32);
    
using TbbMap = tbb::concurrent_hash_map<string, string>;

void BM_TbbConcurrentHashMapBenchInsert(benchmark::State& state)
{
    static TbbMap map;
    for (auto _ : state) {
        benchmark::DoNotOptimize(map.insert({ RandomStr(), RandomStr() }));
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_TbbConcurrentHashMapBenchInsert)->ThreadRange(1, 32);

static void BM_TbbConcurrentHashMapBenchInsertAndFind(benchmark::State& state)
{
    static TbbMap map;
    for (auto _ : state) {
        auto key = RandomStr();
        map.insert({ key, RandomStr() });
        TbbMap::const_accessor ca;
        benchmark::DoNotOptimize(map.find(ca, key));
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_TbbConcurrentHashMapBenchInsertAndFind)->ThreadRange(1, 32);

static void BM_TbbConcurrentHashMapBenchInsertFindAndErase(benchmark::State& state)
{
    static TbbMap map;
    for (auto _ : state) {
        auto key = RandomStr();
        map.insert({ key, RandomStr() });
        {
            TbbMap::const_accessor ca;
            benchmark::DoNotOptimize(map.find(ca, key));
        }
        if (rand() % 100 == 0) {
            map.erase(key);
        }
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_TbbConcurrentHashMapBenchInsertFindAndErase)->ThreadRange(1, 32);

static void BM_TbbConcurrentUnorderedMapBenchInsert(benchmark::State& state)
{
    static tbb::concurrent_unordered_map<string, string> map;
    for (auto _ : state) {
        map.insert({ RandomStr(), RandomStr() });
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_TbbConcurrentUnorderedMapBenchInsert)->ThreadRange(1, 32);

static void BM_TbbConcurrentUnorderedMapBenchInsertAndFind(benchmark::State& state)
{
    static tbb::concurrent_unordered_map<string, string> map;
    for (auto _ : state) {
        auto key = RandomStr();
        map.insert({ key, RandomStr() });
        benchmark::DoNotOptimize(map.find(key));
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_TbbConcurrentUnorderedMapBenchInsertAndFind)->ThreadRange(1, 32);

static void BM_AbslFlatHashMap_insert(benchmark::State& state)
{
    absl::flat_hash_map<string, string> map;
    auto value = RandomStr();
    for (auto _ : state) {
        map.insert({ RandomStr(), value });
        benchmark::ClobberMemory();
    }
}
                                    
BENCHMARK(BM_AbslFlatHashMap_insert);

static void BM_AbslFlatHashMap_insert_find_mix(benchmark::State& state)
{
    absl::flat_hash_map<string, string> map;
    auto value = RandomStr();
    for (auto _ : state) {
        auto key = RandomStr();
        map.insert({ RandomStr(), value });
        benchmark::DoNotOptimize(map.find(key));
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_AbslFlatHashMap_insert_find_mix);

static void BM_AbslNodeHashMap_insert(benchmark::State& state)
{
    absl::node_hash_map<string, string> map;
    auto value = RandomStr();
    for (auto _ : state) {
        map.insert({ RandomStr(), value });
        benchmark::ClobberMemory();
    }
}
                                    
BENCHMARK(BM_AbslNodeHashMap_insert);

static void BM_AbslNodeHashMap_insert_find_mix(benchmark::State& state)
{
    absl::node_hash_map<string, string> map;
    auto value = RandomStr();
    for (auto _ : state) {
        auto key = RandomStr();
        map.insert({ RandomStr(), value });
        benchmark::DoNotOptimize(map.find(key));
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_AbslNodeHashMap_insert_find_mix);

static void BM_StdUnorderedMap_insert(benchmark::State& state)
{
    std::unordered_map<string, string> map;
    auto value = RandomStr();
    for (auto _ : state) {
        map.insert({ RandomStr(), value });
        benchmark::ClobberMemory();
    }
}
                                    
BENCHMARK(BM_StdUnorderedMap_insert);

static void BM_StdUnorderedMap_insert_find_mix(benchmark::State& state)
{
    std::unordered_map<string, string> map;
    auto value = RandomStr();
    for (auto _ : state) {
        auto key = RandomStr();
        map.insert({ RandomStr(), value });
        benchmark::DoNotOptimize(map.find(key));
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_StdUnorderedMap_insert_find_mix);
