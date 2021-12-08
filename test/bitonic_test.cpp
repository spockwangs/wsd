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
    for (int i = 1; i < 4; ++i) {
        int width = (1 << i);
        wsd::Bitonic bitonic(width);
        vector<int> result;
        for (int j = 0; j < 3*width; ++j) {
            result.push_back(bitonic.Traverse(rand() % width));
        }
        CheckResult(result);
    }
}
