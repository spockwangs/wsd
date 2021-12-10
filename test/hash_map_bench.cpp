// Copyright (c) 2021 Tencent, Inc.
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#include <cstdlib>
#include <string>

#include "folly/ConcurrentHashMap.h"
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

class ConcurrentHashMapBench : public wsd::benchmark::Test {
public:
    ConcurrentHashMapBench()
        : m_map(100000, 1000000)
    {}
    
protected:
    folly::ConcurrentHashMap<std::string, std::string> m_map;
};

TEST_CASE(ConcurrentHashMapBench, insert)
{
    m_map.insert(RandomStr(), RandomStr());
}
