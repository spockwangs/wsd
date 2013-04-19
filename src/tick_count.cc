// Copyright (c) 2013 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "tick_count.h"
#include "wsd_exception.h"

namespace wsd {

    TickCount TickCount::now()
    {
        struct timespec tm;
        int err = clock_gettime(CLOCK_MONOTONIC_RAW, &tm);
        if (err)
            throw SyscallException(__FILE__, __LINE__, err);
        return TickCount(tm);
    }

}  // namespace wsd
