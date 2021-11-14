// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "es_test.h"

#include <cstdlib>
#include <new>

int g_throw_counter = -1;
void this_can_throw()
{
    if (g_throw_counter >= 0 && g_throw_counter-- == 0) throw 0;
}

void *operator new(std::size_t size) throw(std::bad_alloc)
{
    try {
        this_can_throw();
    } catch (...) {
        throw std::bad_alloc();
    }
    void *p = std::malloc(size);
    if (!p) throw std::bad_alloc();
    return p;
}

void operator delete(void *ptr) throw()
{
    std::free(ptr);
}

void *operator new[](std::size_t size) throw(std::bad_alloc)
{
    return operator new(size);
}

void operator delete[](void *ptr) throw()
{
    operator delete(ptr);
}

template <class Value, class Operation>
void StrongCheck(const Value &v, const Operation &op)
{
    bool succeeded = false;
    for (long nextThrowCount = 0; !succeeded; ++nextThrowCount) {
        Value duplicate = v;
        try {
            g_throw_counter = nextThrowCount;
            op(duplicate);  // Try the operation
            succeeded = true;
        } catch (...)  // Catch all exceptions
        {
            bool unchanged = duplicate == v;  // Test strong guarantee
            EXPECT_TRUE(unchanged);
        }
        // Specialize as desired for each container type, to check
        // integrity. For example, size() == distance(begin(),end())
        CheckInvariant(v);  // Check any invariant
    }
}
