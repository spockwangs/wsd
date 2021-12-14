// Copyright (c) 2021 Tencent, Inc.
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#include "wsd/bitonic.h"
#include "benchmark/benchmark.h"
#include <atomic>

static void BM_Atomic(benchmark::State& state)
{
    static std::atomic<int> atomic_int;
    for (auto _ : state) {
        benchmark::DoNotOptimize(++atomic_int);
    }
}

BENCHMARK(BM_Atomic)->Threads(1)->Threads(2)->Threads(4)->Threads(8)->Threads(16)->Threads(32)->Threads(64)->Threads(128);

static void BM_Bitonic(benchmark::State& state)
{
    const int kWidth = 1 << 5;
    static wsd::Bitonic bitonic(kWidth);
    for (auto _ : state) {
        benchmark::DoNotOptimize(bitonic.Traverse(state.thread_index() % kWidth));
    }
}

BENCHMARK(BM_Bitonic)->Threads(1)->Threads(2)->Threads(4)->Threads(8)->Threads(16)->Threads(32)->Threads(64)->Threads(128);
BENCHMARK_MAIN();

