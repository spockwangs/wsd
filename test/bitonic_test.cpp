// Copyright (c) 2021 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "wsd/bitonic.h"

#include <map>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <gtest/gtest.h>
#include <iostream>
#include <thread>

using namespace std;

void CheckResult(const vector<int>& result)
{
    map<int, int> m;
    for (auto i : result) {
        m[i]++;
    }
    auto it = m.begin();
    int n = it->second;
    ++it;
    for (; it != m.end(); ++it) {
        if (n != it->second) {
            --n;
            break;
        }
    }
    for (; it != m.end(); ++it) {
        EXPECT_EQ(it->second, n);
    }
}
        
TEST(Bitonic, basic)
{
    for (int i = 1; i < 20; ++i) {
        int width = (1 << i);
        wsd::Bitonic bitonic(width);
        vector<int> result;
        for (int j = 0; j < 3*width; ++j) {
            result.push_back(bitonic.Traverse(rand() % width));
        }
        CheckResult(result);
    }
}

TEST(Bitonic, multithread)
{
    const int width = (1 << 10);
    wsd::Bitonic bitonic(width);
    vector<thread> threads;
    vector<vector<int>> result_per_thread;
    result_per_thread.resize(width);
    for (int i = 0; i < width; ++i) {
        threads.emplace_back(
                [&, i] {
                    for (int j = 0; j < 100; ++j) {
                        result_per_thread[i].push_back(bitonic.Traverse(i));
                    }
                });
    }
    for (auto& t : threads) {
        t.join();
    }
    vector<int> result;
    for (const auto& v : result_per_thread) {
        result.insert(result.end(), v.begin(), v.end());
    }
    CheckResult(result);
}
