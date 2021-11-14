// Copyright (c) 2019 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "rate_limiter.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <thread>

using namespace std;

namespace {

int64_t GetNowMicros()
{
    int64_t now_micros =
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch())
                    .count();
    return now_micros;
}

bool DoubleEqual(double f1, double f2)
{
    const double epsilon = 1e-5;
    if (std::abs(f1 - f2) < epsilon) {
        return true;
    }
    return false;
}

}  // namespace

namespace wsd {

RateLimiter::RateLimiter(int permits_per_sec, double max_burst_seconds) : m_max_burst_seconds(max_burst_seconds)
{
    assert(permits_per_sec > 0);
    assert(max_burst_seconds > 0.0);
    SetRate(permits_per_sec);
}

void RateLimiter::SetRate(int permits_per_sec)
{
    assert(permits_per_sec > 0);
    std::lock_guard<std::mutex> lock(m_mutex);
    m_stable_interval_micros = static_cast<double>(1000000) / permits_per_sec;
    int64_t now_micros = GetNowMicros();
    Resync(now_micros);
    double old_max_permits = m_max_permits;
    m_max_permits = m_max_burst_seconds * permits_per_sec;
    if (DoubleEqual(old_max_permits, 0.0)) {
        m_stored_permits = 0.0;
    } else {
        m_stored_permits = m_stored_permits * m_max_permits / old_max_permits;
    }
}

void RateLimiter::Acquire(int permits)
{
    assert(permits > 0);
    int64_t now_micros = GetNowMicros();
    int64_t wait_micros = ReserveAndGetWaitLength(permits, now_micros, -1);
    if (wait_micros > 0) {
        std::this_thread::sleep_for(std::chrono::microseconds(wait_micros));
    }
}

int64_t RateLimiter::AcquireDelay(int permits)
{
    int64_t now_micros = GetNowMicros();
    return ReserveAndGetWaitLength(permits, now_micros, -1);
}

bool RateLimiter::TryAcquire(int permits, int64_t timeout_micros)
{
    int64_t now_micros = GetNowMicros();
    int64_t wait_micros = ReserveAndGetWaitLength(permits, now_micros, timeout_micros);
    if (wait_micros < 0) {
        return false;
    }
    if (wait_micros == 0) {
        return true;
    }

    assert(wait_micros <= timeout_micros);
    std::this_thread::sleep_for(std::chrono::microseconds(wait_micros));
    return true;
}

int64_t RateLimiter::ReserveAndGetWaitLength(int permits, int64_t now_micros, int64_t timeout_micros)
{
    int64_t next_free_micros = 0;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        Resync(now_micros);

        next_free_micros = m_next_free_micros;
        double stored_permits_to_spend = std::min(static_cast<double>(permits), m_stored_permits);
        double fresh_permits = permits - stored_permits_to_spend;
        double wait_micros = fresh_permits * m_stable_interval_micros;
        if (timeout_micros < 0 || next_free_micros <= now_micros + timeout_micros) {
            m_stored_permits -= stored_permits_to_spend;
            m_next_free_micros += wait_micros;
        } else {
            return -1;
        }
    }
    if (next_free_micros > now_micros) {
        return next_free_micros - now_micros;
    }
    return 0;
}

void RateLimiter::Resync(int64_t now_micros)
{
    if (now_micros > m_next_free_micros) {
        double new_permits = (now_micros - m_next_free_micros) / m_stable_interval_micros;
        m_stored_permits = std::min(m_max_permits, m_stored_permits + new_permits);
        m_next_free_micros = now_micros;
    }
}

}  // namespace wsd
