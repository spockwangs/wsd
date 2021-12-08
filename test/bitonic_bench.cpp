// Copyright (c) 2021 Tencent, Inc.
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#include "wsd/bitonic.h"
#include "wsd/benchmark.h"
#include "wsd/util.h"
#include <cstdlib>
#include <atomic>

using namespace std;

class BitonicBench : public wsd::benchmark::Test {
public:
    BitonicBench()
        : m_width(1 << 5),
          m_bitonic(m_width),
          m_int(0)
    {}
    
protected:
    const int m_width;
    wsd::Bitonic m_bitonic;
    std::atomic<int> m_int;
};

TEST_CASE(BitonicBench, basic)
{
    m_bitonic.Traverse(wsd::ThreadId() % m_width);
    return 0;
}

TEST_CASE(BitonicBench, atomic)
{
    ++m_int;
    return 0;
}
