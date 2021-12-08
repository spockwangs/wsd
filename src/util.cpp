// Copyright (c) 2021 Tencent, Inc.
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#include "util.h"
#include <atomic>

using namespace std;

namespace wsd {

int ThreadId()
{
    static atomic<int> counter(0);
    thread_local int tid = counter++;
    return tid;
}

}  // namespace wsd
