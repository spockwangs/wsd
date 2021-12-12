// Copyright (c) 2021 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "wsd/benchmark.h"
#include "wsd/detail/benchmark_detail.h"
#include <cassert>
#include <csignal>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <exception>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include "wsd/rate_limiter.h"
#include <gflags/gflags.h>

DEFINE_int32(duration, 10, "duration in seconds");
DEFINE_int32(concurrency, 1, "concurrency level");
DEFINE_bool(heartbeat, true, "heartbeat or not");
DEFINE_int32(max_qps, 0, "max qps");
DEFINE_string(test, "", "Run the specified test, or all tests if empty");
DEFINE_bool(list_tests, false, "list all test cases");
DEFINE_bool(verbose, false, "");

using namespace std;

namespace wsd {
namespace benchmark {
namespace detail {

void Benchmark::Init(const Options& opts)
{
    m_options = opts;
}

void Benchmark::Start(const std::function<int()>& op)
{
    if (!op) {
        throw std::invalid_argument("test operation is not set");
    }
    m_op = op;
    m_should_quit = false;
    m_thread = std::thread(&Benchmark::Run, this);
}

void Benchmark::WaitForDone()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void Benchmark::Run()
{
    RateLimiter rate_limiter(1);
    if (m_options.max_qps > 0) {
        rate_limiter.SetRate(m_options.max_qps);
    }

    int num_of_threads = m_options.concurrency;
    vector<vector<uint32_t>> stats_per_thread(num_of_threads);
    atomic<int> n_succ_requests(0);
    atomic<int> n_done_requests(0);
    atomic<int> thread_exit_cnt(0);
    atomic<int> failed_requests(0);
    int actual_concurrency = 0;
    condition_variable cv_thread_exit;
    
    auto send_reqs = [&] (int i) {
                         try {
                             while (!m_should_quit) {
                                 if (m_options.max_qps > 0) {
                                     rate_limiter.Acquire(1);
                                 }
                                 auto start_time = std::chrono::steady_clock::now();
                                 int err = [this]() -> int {
                                               try {
                                                   int ret = m_op();
                                                   if (FLAGS_verbose && ret) {
                                                       cerr << "ret=" << ret << endl;
                                                   }
                                                   return ret;
                                               } catch (...) {
                                                   return -1;
                                               }
                                           }();
                                 if (err) {
                                     ++failed_requests;
                                 } else {
                                     ++n_succ_requests;
                                 }
                                 ++n_done_requests;
                                 auto end_time = std::chrono::steady_clock::now();
                                 auto latency_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                                 stats_per_thread[i].push_back(latency_ms.count());
                             }
                         } catch (...) {
                         }
                     };
    
    // Create multiple threads to send requests.
    auto start_time = std::chrono::steady_clock::now();
    vector<thread> threads;
    for (int i = 0; i < num_of_threads; ++i) {
        try {
            threads.emplace_back([&, i]() {
                send_reqs(i);
                thread_exit_cnt.fetch_add(1, std::memory_order_release);
                cv_thread_exit.notify_one();
            });
            ++actual_concurrency;
        } catch (...) {
        }
    }

    // Wait to finish.
    {
        std::mutex m;
        auto predicate = [&]() {
                             auto now = std::chrono::steady_clock::now();
                             auto elapse_seconds = chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
                             if (elapse_seconds >= m_options.duration_seconds) {
                                 return true;
                             }
                             return thread_exit_cnt.load(std::memory_order_acquire) == actual_concurrency;
                         };
        std::unique_lock<std::mutex> lock(m);
        if (m_options.heartbeat) {
            while (!cv_thread_exit.wait_for(lock, std::chrono::seconds(10), predicate)) {
                cout << "Complete " << n_done_requests << " requests ..." << endl;
            }
        } else {
            cv_thread_exit.wait(lock, predicate);
        }
    }
    m_should_quit = true;
    for (auto& t : threads) {
        t.join();
    }

    // Collect statistics.
    auto end_time = std::chrono::steady_clock::now();
    double time_taken = chrono::duration_cast<chrono::duration<double>>(end_time - start_time).count();

    m_stat.concurrency = m_options.concurrency;
    m_stat.actual_concurrency = actual_concurrency;
    m_stat.time_taken = time_taken;
    m_stat.complete_requests = n_done_requests;
    m_stat.failed_requests = failed_requests;
    m_stat.requests_per_second = m_stat.complete_requests / m_stat.time_taken;
    if (m_stat.complete_requests > 0) {
        vector<uint32_t> latencies;
        uint64_t sum_latency = 0;
        for (const auto& s : stats_per_thread) {
            latencies.insert(latencies.end(), s.begin(), s.end());
            for (auto l : s) {
                sum_latency += l;
            }
        }
        m_stat.time_per_request_per_thread = static_cast<double>(sum_latency) / latencies.size();
        if (FLAGS_verbose) {
            cout << "first request latency: " << latencies[0] << endl;
        }
        sort(latencies.begin(), latencies.end());

        assert(!latencies.empty());
        uint32_t max_latency = latencies.back();
        for (auto p = latencies.crbegin(); p != latencies.crend(); ++p) {
            if (*p == max_latency) {
                ++m_stat.num_of_longest_requests;
            } else {
                break;
            }
        }
            
        size_t idx = 0;
        for (int perc : { 50, 66, 75, 80, 90, 95, 98, 99, 100 }) {
            if (perc >= 100) {
                m_stat.percentiles[idx] = latencies.back();
            } else {
                m_stat.percentiles[idx] = latencies[perc * latencies.size() / 100];
            }
            ++idx;
        }
    }
}

void Benchmark::Stop()
{
    m_should_quit = true;
}

Benchmark::Stat Benchmark::GetResult()
{
    return m_stat;
}

void Benchmark::PrintResult()
{
    cout << "Set concurrency Level: " << m_stat.concurrency << endl;
    cout << "Acutual concurrency level: " << m_stat.actual_concurrency << endl;
    cout << "Time taken for tests: " << m_stat.time_taken << "s" << endl;
    cout << "Complete requests: " << m_stat.complete_requests << endl;
    double fail_percents = static_cast<double>(m_stat.failed_requests * 100) / m_stat.complete_requests;
    cout << "Failed requests: " << m_stat.failed_requests << " (" << fail_percents << "%)" << endl;
    cout << "Requests per second: " << m_stat.requests_per_second << " [#/s] (mean)" << endl;
    cout << "Time per request per thread: " << m_stat.time_per_request_per_thread << " [ms] (mean)" << endl;
    cout << "Percentage of the requests served within a certain time (ms)" << endl;
    size_t idx = 0;
    for (int percentile : { 50, 66, 75, 80, 90, 95, 98, 99, 100 }) {
        if (percentile == 100) {
            cout << " " << percentile << "%    " << m_stat.percentiles[idx]
                 << " (" << m_stat.num_of_longest_requests << " longest requests)" << endl;
        } else {
            cout << "  " << percentile << "%    " << m_stat.percentiles[idx] << endl;
        }
        ++idx;
    }
}

Benchmark::Options MakeBenchOptions()
{
    Benchmark::Options result;
    if (FLAGS_duration <= 0) {
        throw std::invalid_argument("non-positive number of duration: " + to_string(FLAGS_duration));
    }
    result.duration_seconds = FLAGS_duration;    

    if (FLAGS_concurrency < 0) {
        throw std::invalid_argument("non-positive concurrency: " + to_string(FLAGS_concurrency));
    }
    result.concurrency = FLAGS_concurrency;
    result.heartbeat = FLAGS_heartbeat;

    if (FLAGS_max_qps < 0) {
        throw std::invalid_argument("negative max_qps: " + to_string(FLAGS_max_qps));
    }
    result.max_qps = FLAGS_max_qps;
    return result;
}
            
BenchmarkManager& BenchmarkManager::GetInstance()
{
    static BenchmarkManager instance{};
    return instance;
}

void BenchmarkManager::Init(int* argc, char*** argv)
{
    ::google::ParseCommandLineFlags(argc, argv, false);
    m_bench.Init(MakeBenchOptions());
    
    // Start a thread to process signal SIGINT.
    sigset_t sig_set;
    sigemptyset(&sig_set);
    sigaddset(&sig_set, SIGINT);
    int err = pthread_sigmask(SIG_BLOCK, &sig_set, nullptr);
    if (err) {
        throw std::logic_error("pthread_sigmask() failed, returned " + to_string(err));
    }
    std::thread([this, sig_set] () {
                    // This thread will be killed when the process exits, so we don't care
                    // how to exit this thread.
                    for (;;) {
                        int sig;
                        int err = sigwait(&sig_set, &sig);
                        if (err) {
                            throw std::logic_error("sigwait() failed, returned " + to_string(err));
                        }
                        if (sig == SIGINT) {
                            this->m_bench.Stop();
                        }
                    }
                }).detach();
}

void* BenchmarkManager::Register(
        const std::string& class_name,
        const std::string& method_name,
        TestFactory* test_factory)
{
    m_tests.insert({ class_name + "." + method_name,
                     std::shared_ptr<TestFactory>(test_factory) });
    return nullptr;
}
        
int BenchmarkManager::RunTests()
{
    if (FLAGS_list_tests) {
        return ListAllTests();
    } else if (FLAGS_test.empty()) {
        return RunAllTests();
    }
    return RunOneTest(FLAGS_test);
}

int BenchmarkManager::ListAllTests()
{
    for (const auto& p : m_tests) {
        cout << p.first << endl;
    }
    return 0;
}

int BenchmarkManager::RunAllTests()
{
    try {
        for (const auto& p : m_tests) {
            cout << "Testing " << p.first << " ... " << flush;
            unique_ptr<Test> test_case_ptr(p.second->CreateTest());
            test_case_ptr->SetUp();
            m_bench.Start(bind(&benchmark::Test::TestBody, test_case_ptr.get()));
            m_bench.WaitForDone();
            test_case_ptr->TearDown();
            cout << "done." << endl;            
            m_bench.PrintResult();
            cout << endl;
        }
    } catch (exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
        
int BenchmarkManager::RunOneTest(const string& test_name)
{
    try {
        auto it = m_tests.find(test_name);
        if (it != m_tests.end()) {
            cout << "Testing " << it->first << " ... " << flush;
            unique_ptr<benchmark::Test> test_case_ptr(it->second->CreateTest());
            test_case_ptr->SetUp();
            m_bench.Start(bind(&benchmark::Test::TestBody, test_case_ptr.get()));
            m_bench.WaitForDone();
            test_case_ptr->TearDown();
            cout << "done." << endl;
            m_bench.PrintResult();
            cout << endl;
            return 0;
        }
        cerr << "no such test: " << test_name << endl;
    } catch (exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
        
BenchmarkManager::BenchmarkManager()
{
}

}  // namespace detail

void Init(int* argc, char*** argv)
{
    detail::BenchmarkManager::GetInstance().Init(argc, argv);
}

int RunTests()
{
    return detail::BenchmarkManager::GetInstance().RunTests();
}

}  // namespace benchmark
}  // namespace wsd


