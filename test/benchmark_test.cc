// Copyright (c) 2021 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "wsd/benchmark.h"
#include <thread>
#include <chrono>

class MyTest : public wsd::benchmark::Test {
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_CASE(MyTest, Sleep)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
