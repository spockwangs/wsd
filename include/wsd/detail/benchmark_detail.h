// Copyright (c) 2021 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <memory>
#include <thread>
#include <atomic>

namespace wsd {
namespace benchmark {

class Test;

namespace detail {

class Benchmark {
public:
    struct Options {
        int duration_seconds;
        int concurrency;
        bool heartbeat;
        int max_qps;

        Options()
            : duration_seconds(10),
              concurrency(0),
              heartbeat(false),
              max_qps(0)
        {}
    };

    struct Stat {
        int concurrency = 0;
        int actual_concurrency = 0;
        double time_taken = 0.0;
        int complete_requests = 0;
        int failed_requests = 0;
        double requests_per_second = 0.0;
        double time_per_request_per_thread = 0.0;
        // Percentiles: 55, 66, 75, 80, 90, 95, 98, 99, 100.
        uint32_t percentiles[9] = { 0 };
        int num_of_longest_requests = 0;
    };

    Benchmark() = default;

    void Init(const Options& opts);

    void Start(const std::function<int()>& op);
    
    void Stop();

    void WaitForDone();
    
    Stat GetResult();

    void PrintResult();

private:
    void Run();

    std::function<int()> m_op;
    Options m_options;
    Stat m_stat;
    std::atomic<bool> m_should_quit{false};
    std::thread m_thread;
};

class TestFactory {
public:
    TestFactory()
    {}

    virtual ~TestFactory()
    {}

    virtual benchmark::Test* CreateTest() = 0;
};

template <typename TestClass>
class TestFactoryImpl : public TestFactory {
public:
    virtual benchmark::Test* CreateTest()
    {
        return new TestClass();
    }
};

class BenchmarkManager {
public:
    static BenchmarkManager& GetInstance();

    void Init(int* argc, char*** argv);

    void* Register(const std::string& class_name,
                   const std::string& method_name,
                   TestFactory* test_factory);
        
    int RunTests();

private:
    BenchmarkManager();

    BenchmarkManager(BenchmarkManager&) = delete;
    void operator=(BenchmarkManager&) = delete;

    int ListAllTests();

    int RunAllTests();

    int RunOneTest(const std::string& test_name);
            
    Benchmark m_bench;
    std::map<std::string, std::shared_ptr<TestFactory>> m_tests;
};

}  // namespace detail
}  // namespace benchmark
}
