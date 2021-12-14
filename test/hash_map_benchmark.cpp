// Copyright (c) 2021 Tencent, Inc.
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#include <cstdlib>
#include <string>

#include "folly/concurrency/ConcurrentHashMap.h"
#include "folly/AtomicHashMap.h"
#include "folly/AtomicUnorderedMap.h"
#include "tbb/concurrent_hash_map.h"
#include "tbb/concurrent_unordered_map.h"
#include "benchmark/benchmark.h"

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
        map.insert(RandomStr(), RandomStr());
    }
}
    
BENCHMARK(BM_FollyConcurrentHashMapInsert)->ThreadRange(1, 32);

static void BM_FollyConcurrentHashMapInsertAndFind(benchmark::State& state)
{
    static folly::ConcurrentHashMap<std::string, std::string> map;
    for (auto _ : state) {
        auto key = RandomStr();
        map.insert(key, RandomStr());
        map.find(key);
    }
}

BENCHMARK(BM_FollyConcurrentHashMapInsertAndFind)->ThreadRange(1, 32);

static void BM_FollyConcurrentHashMapBenchInsertFindAndErase(benchmark::State& state)
{
    static folly::ConcurrentHashMap<std::string, std::string> map;
    for (auto _ : state) {
        auto key = RandomStr();
        map.insert(key, RandomStr());
        map.find(key);
        if (rand() % 100 == 0) {
            map.erase(key);
        }
    }
}

BENCHMARK(BM_FollyConcurrentHashMapBenchInsertFindAndErase)->ThreadRange(1, 32);

static void BM_FollyAtomicHashMapBenchInsert(benchmark::State& state)
{
    static folly::AtomicHashMap<int, string> map(10000000);
    for (auto _ : state) {
        map.insert(rand(), RandomStr());
    }
}

BENCHMARK(BM_FollyAtomicHashMapBenchInsert)->ThreadRange(1, 32);

static void BM_FollyAtomicHashMapInsertAndFind(benchmark::State& state)
{
    static folly::AtomicHashMap<int, string> map(10000000);
    for (auto _ : state) {
        auto key = rand();
        map.insert(key, RandomStr());
        map.find(key);
    }
}

BENCHMARK(BM_FollyAtomicHashMapInsertAndFind)->ThreadRange(1, 32);

static void BM_FollyAtomicHashMapBenchInsertFindAndErase(benchmark::State& state)
{
    static folly::AtomicHashMap<int, string> map(10000000);
    for (auto _ : state) {
        auto key = rand();
        map.insert(key, RandomStr());
        map.find(key);
        if (rand() % 100 == 0) {
            map.erase(key);
        }
    }
}

BENCHMARK(BM_FollyAtomicHashMapBenchInsertFindAndErase)->ThreadRange(1, 32);

static void BM_FollyAtomicUnorderedInsertMapInsert(benchmark::State& state)
{
    static folly::AtomicUnorderedInsertMap<string, string> map(100000000);
    for (auto _ : state) {
        map.findOrConstruct(RandomStr(), [] (void* p) {
                                               new (p) std::string(RandomStr());
                                           });
    }
}
                                   
BENCHMARK(BM_FollyAtomicUnorderedInsertMapInsert)->ThreadRange(1, 32);

static void BM_FollyAtomicUnorderedInsertMapInsertAndFind(benchmark::State& state)
{
    static folly::AtomicUnorderedInsertMap<string, string> map(100000000);
    for (auto _ : state) {
        auto key = RandomStr();
        map.findOrConstruct(key, [=] (void* p) {
                                       new (p) std::string(RandomStr());
                                   });
        map.find(key);
    }
}

BENCHMARK(BM_FollyAtomicUnorderedInsertMapInsertAndFind)->ThreadRange(1, 32);
    
using TbbMap = tbb::concurrent_hash_map<string, string>;

void BM_TbbConcurrentHashMapBenchInsert(benchmark::State& state)
{
    static TbbMap map;
    for (auto _ : state) {
        map.insert({ RandomStr(), RandomStr() });
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
        map.find(ca, key);
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
            map.find(ca, key);
        }
        if (rand() % 100 == 0) {
            map.erase(key);
        }
    }
}

BENCHMARK(BM_TbbConcurrentHashMapBenchInsertFindAndErase)->ThreadRange(1, 32);

static void BM_TbbConcurrentUnorderedMapBenchInsert(benchmark::State& state)
{
    static tbb::concurrent_unordered_map<string, string> map;
    for (auto _ : state) {
        map.insert({ RandomStr(), RandomStr() });
    }
}

BENCHMARK(BM_TbbConcurrentUnorderedMapBenchInsert)->ThreadRange(1, 32);

static void BM_TbbConcurrentUnorderedMapBenchInsertAndFind(benchmark::State& state)
{
    static tbb::concurrent_unordered_map<string, string> map;
    for (auto _ : state) {
        auto key = RandomStr();
        map.insert({ key, RandomStr() });
        map.find(key);
    }
}

BENCHMARK(BM_TbbConcurrentUnorderedMapBenchInsertAndFind)->ThreadRange(1, 32);
