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
#include "wsd/benchmark.h"

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

class FollyConcurrentHashMapBench : public wsd::benchmark::Test {
public:
    FollyConcurrentHashMapBench()
        : m_map(100000)
    {}
    
protected:
    folly::ConcurrentHashMap<std::string, std::string> m_map;
};

TEST_CASE(FollyConcurrentHashMapBench, insert)
{
    m_map.insert(RandomStr(), RandomStr());
    return 0;
}

TEST_CASE(FollyConcurrentHashMapBench, insertAndFind)
{
    auto key = RandomStr();
    m_map.insert(key, RandomStr());
    m_map.find(key);
    return 0;
}

TEST_CASE(FollyConcurrentHashMapBench, insertFindAndErase)
{
    auto key = RandomStr();
    m_map.insert(key, RandomStr());
    m_map.find(key);
    if (rand() % 100 == 0) {
        m_map.erase(key);
    }
    return 0;
}

class FollyAtomicHashMapBench : public wsd::benchmark::Test {
public:
    FollyAtomicHashMapBench()
        : m_map(10000000)
    {}
    
protected:
    folly::AtomicHashMap<int, string> m_map;
};

TEST_CASE(FollyAtomicHashMapBench, insert)
{
    m_map.insert(rand(), RandomStr());
    return 0;
}

TEST_CASE(FollyAtomicHashMapBench, insertAndFind)
{
    auto key = rand();
    m_map.insert(key, RandomStr());
    m_map.find(key);
    return 0;
}

TEST_CASE(FollyAtomicHashMapBench, insertFindAndErase)
{
    auto key = rand();
    m_map.insert(key, RandomStr());
    m_map.find(key);
    if (rand() % 100 == 0) {
        m_map.erase(key);
    }
    return 0;
}

class FollyAtomicUnorderedInsertMapBench : public wsd::benchmark::Test {
public:
    FollyAtomicUnorderedInsertMapBench()
        : m_map(100000000)
    {}
    
protected:
    folly::AtomicUnorderedInsertMap<string, string> m_map;
};

TEST_CASE(FollyAtomicUnorderedInsertMapBench, insert)
{
    m_map.findOrConstruct(RandomStr(), [] (void* p) {
                                           new (p) std::string(RandomStr());
                                       });
    return 0;
}
                                   
TEST_CASE(FollyAtomicUnorderedInsertMapBench, insertAndFind)
{
    auto key = RandomStr();
    m_map.findOrConstruct(key, [=] (void* p) {
                                   new (p) std::string(RandomStr());
                               });
    m_map.find(key);
    return 0;
}

class TbbConcurrentHashMapBench : public wsd::benchmark::Test {
public:
    TbbConcurrentHashMapBench()
        : m_map(100000)
    {}

protected:
    using Map = tbb::concurrent_hash_map<string, string>;
    Map m_map;
};
    
TEST_CASE(TbbConcurrentHashMapBench, insert)
{
    m_map.insert({ RandomStr(), RandomStr() });
    return 0;
}

TEST_CASE(TbbConcurrentHashMapBench, insertAndFind)
{
    auto key = RandomStr();
    m_map.insert({ key, RandomStr() });
    Map::const_accessor ca;
    m_map.find(ca, key);
    return 0;
}

TEST_CASE(TbbConcurrentHashMapBench, insertFindAndErase)
{
    auto key = RandomStr();
    m_map.insert({ key, RandomStr() });
    {
        Map::const_accessor ca;
        m_map.find(ca, key);
    }
    if (rand() % 100 == 0) {
        m_map.erase(key);
    }
    return 0;
}

class TbbConcurrentUnorderedMapBench : public wsd::benchmark::Test {
public:
    TbbConcurrentUnorderedMapBench()
        : m_map(100000)
    {}

protected:
    using Map = tbb::concurrent_unordered_map<string, string>;
    Map m_map;
};
    
TEST_CASE(TbbConcurrentUnorderedMapBench, insert)
{
    m_map.insert({ RandomStr(), RandomStr() });
    return 0;
}

TEST_CASE(TbbConcurrentUnorderedMapBench, insertAndFind)
{
    auto key = RandomStr();
    m_map.insert({ key, RandomStr() });
    m_map.find(key);
    return 0;
}
