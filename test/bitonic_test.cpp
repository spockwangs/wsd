// Copyright (c) 2021 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "wsd/bitonic.h"

#include <algorithm>
#include <gtest/gtest.h>

using namespace std;

void CheckResult(vector<int>& result, int n)
{
    EXPECT_EQ(result.size(), n);
    sort(result.begin(), result.end());
    for (size_t i = 0; i < result.size(); ++i) {
        EXPECT_EQ(result[i], i);
    }
}
        
TEST(Bitonic, basic)
{
    wsd::Bitonic bitonic(2);
    vector<int> result;
    result.push_back(bitonic.Traverse(0));
    result.push_back(bitonic.Traverse(1));
    CheckResult(result, 2);
}
