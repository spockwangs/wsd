// Copyright (c) 2019 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include <cstdint>
#include <mutex>

namespace wsd {

class RateLimiter final {
public:
    RateLimiter(int permits_per_sec, double max_burst_seconds = 1);

    // |permits_per_sec| may not exceed 1000000.
    void SetRate(int permits_per_sec);

    void Acquire(int permits);

    int64_t AcquireDelay(int permits);

    bool TryAcquire(int permits, int64_t timeout_micros);

private:
    // @param timeout_micros maximum time to wait. Negative values means forever.
    int64_t ReserveAndGetWaitLength(int permits, int64_t now_micros, int64_t timeout_micros);

    void Resync(int64_t now_micros);

    const double m_max_burst_seconds = 0;
    std::mutex m_mutex;
    double m_stable_interval_micros = 0;
    double m_max_permits = 0;
    double m_next_free_micros = 0;
    double m_stored_permits = 0;
};

}  // namespace wsd
