// Copyright (c) 2021 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include <time.h>
#include "wsd/combining_tree.h"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;

int main(int argc, char** argv)
{
    const int kThreads = 100;
    wsd::CombiningTree tree(kThreads*2);
    vector<thread> threads;
    int start_time = time(nullptr);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < 1000000; ++j) {
                tree.GetAndIncrement();
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    int end_time = time(nullptr);
    cout << "latency=" << (end_time - start_time) << endl;
    cout << "count=" << tree.Get() << endl;

    mutex mu;
    int counter = 0;
    threads.clear();
    start_time = time(nullptr);
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < 1000000; ++j) {
                unique_lock<mutex> lock(mu);
                ++counter;
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    end_time = time(nullptr);
    cout << "latency=" << (end_time - start_time) << endl;
    return 0;
}
