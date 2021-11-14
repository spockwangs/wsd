// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//
// See http://www.boost.org/community/exception_safety.html

#ifndef __ES_TEST_H__
#define __ES_TEST_H__

#include "gtest/gtest.h"

extern int g_throw_counter;

void this_can_throw();

#define ES_NO_THROW(expression)                                          \
    {                                                                    \
        bool succeeded = false;                                          \
        for (int next_throw_count = 0; !succeeded; next_throw_count++) { \
            g_throw_counter = next_throw_count;                          \
            try {                                                        \
                expression;                                              \
                succeeded = true;                                        \
            } catch (...) {                                              \
                FAIL();                                                  \
            }                                                            \
        }                                                                \
        g_throw_counter = -1;                                            \
    }

#define ES_MAY_THROW(expression, exception_type)                         \
    {                                                                    \
        bool succeeded = false;                                          \
        for (int next_throw_count = 0; !succeeded; next_throw_count++) { \
            g_throw_counter = next_throw_count;                          \
            try {                                                        \
                expression;                                              \
                succeeded = true;                                        \
            } catch (exception_type&) {                                  \
                SUCCEED();                                               \
            } catch (...) {                                              \
                FAIL();                                                  \
            }                                                            \
        }                                                                \
        g_throw_counter = -1;                                            \
    }

#define ES_MAY_THROW2(expression, exception_type1, exception_type2)      \
    {                                                                    \
        bool succeeded = false;                                          \
        for (int next_throw_count = 0; !succeeded; next_throw_count++) { \
            g_throw_counter = next_throw_count;                          \
            try {                                                        \
                expression;                                              \
                succeeded = true;                                        \
            } catch (exception_type1&) {                                 \
                SUCCEED();                                               \
            } catch (exception_type2&) {                                 \
                SUCCEED();                                               \
            } catch (...) {                                              \
                FAIL();                                                  \
            }                                                            \
        }                                                                \
        g_throw_counter = -1;                                            \
    }

#define ES_MAY_THROW3(expression, exception_type1, exception_type2, exception_type3) \
    {                                                                                \
        bool succeeded = false;                                                      \
        for (int next_throw_count = 0; !succeeded; next_throw_count++) {             \
            g_throw_counter = next_throw_count;                                      \
            try {                                                                    \
                expression;                                                          \
                succeeded = true;                                                    \
            } catch (exception_type1&) {                                             \
                SUCCEED();                                                           \
            } catch (exception_type2&) {                                             \
                SUCCEED();                                                           \
            } catch (exception_type3&) {                                             \
                SUCCEED();                                                           \
            } catch (...) {                                                          \
                FAIL();                                                              \
            }                                                                        \
        }                                                                            \
        g_throw_counter = -1;                                                        \
    }

#define ES_ANY_THROW(expression)                                         \
    {                                                                    \
        bool succeeded = false;                                          \
        for (int next_throw_count = 0; !succeeded; next_throw_count++) { \
            g_throw_counter = next_throw_count;                          \
            try {                                                        \
                expression;                                              \
                succeeded = true;                                        \
            } catch (...) {                                              \
                SUCCEED();                                               \
            }                                                            \
        }                                                                \
        g_throw_counter = -1;                                            \
    }

struct TestClass {
    TestClass(int v = 0) : p((this_can_throw(), new int(v)))
    {
    }

    TestClass(const TestClass& rhs) : p((this_can_throw(), new int(*rhs.p)))
    {
    }

    const TestClass& operator=(const TestClass& rhs)
    {
        this_can_throw();
        *p = *rhs.p;
        return *this;
    }

    ~TestClass()
    {
        delete p;
    }

    void run() const
    {
        this_can_throw();
    }

    int* p;
};

inline bool operator<(const TestClass& lhs, const TestClass& rhs)
{
    return *lhs.p < *rhs.p;
}

inline bool operator==(const TestClass& lhs, const TestClass& rhs)
{
    return *lhs.p == *rhs.p;
}

// Specialize this for particular container if such a check is available.
template <typename Container>
void checkInvariant(const Container&)
{
}

template <class Value, class Operation>
void strongCheck(const Value& v, const Operation& op)
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
            g_throw_counter = -1;
            bool unchanged = duplicate == v;  // Test strong guarantee
            EXPECT_TRUE(unchanged);
        }
        // Specialize as desired for each container type, to check
        // integrity. For example, size() == distance(begin(),end())
        checkInvariant(v);  // Check any invariant
    }
    g_throw_counter = -1;
}

template <class Value, class Operation>
void weakCheck(const Value& v, const Operation& op)
{
    bool succeeded = false;
    for (long nextThrowCount = 0; !succeeded; ++nextThrowCount) {
        Value dup = v;
        try {
            g_throw_counter = nextThrowCount;
            op(dup);  // Try the operation
            succeeded = true;
        } catch (...) {
        }
        g_throw_counter = -1;
        // Specialize as desired for each container type, to check
        // integrity. For example, size() == distance(begin(),end())
        checkInvariant(v);  // Check any invariant
    }
    g_throw_counter = -1;
}

template <class Value, class Operation>
void nothrowCheck(const Value& v, const Operation& op)
{
    bool succeeded = false;
    for (long nextThrowCount = 0; !succeeded; ++nextThrowCount) {
        Value dup = v;
        try {
            g_throw_counter = nextThrowCount;
            op(dup);  // Try the operation
            succeeded = true;
        } catch (...) {
            FAIL() << "should not throw";
        }
        g_throw_counter = -1;
        // Specialize as desired for each container type, to check
        // integrity. For example, size() == distance(begin(),end())
        checkInvariant(v);  // Check any invariant
    }
    g_throw_counter = -1;
}

#endif  // __ES_TEST_H__
