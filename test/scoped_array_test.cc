// -*- coding: utf-8-unix; -*-
// Copyright (c) 2012 Tencent, Inc.
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#include "scoped_array.h"
#include "gtest/gtest.h"
#include "es_test.h"

TEST(scoped_array, constructor)
{
    wsd::ScopedArray<int> ptr;
    EXPECT_TRUE(ptr.get() == NULL);
    EXPECT_FALSE(ptr);

    int *pi = new int[10];
    wsd::ScopedArray<int> ptr2(pi);
    ASSERT_EQ(pi, ptr2.get());
    for (size_t i = 0; i < 10; i++) {
        EXPECT_EQ(pi[i], ptr2[i]);
    }
    EXPECT_TRUE(ptr2);
}

TEST(scoped_array, reset)
{
    wsd::ScopedArray<int> ptr(new int[10]);
    ASSERT_TRUE(ptr);
    ptr[3] = 0xdead;
    EXPECT_EQ(0xdead, ptr[3]);

    ptr.reset();
    EXPECT_FALSE(ptr);
    EXPECT_TRUE(ptr.get() == NULL);

    int *pi = new int[10];
    ptr.reset(pi);
    ASSERT_TRUE(ptr);
    for (size_t i = 0; i < 10; i++)
        ASSERT_EQ(pi[i], ptr[i]);
    ASSERT_EQ(pi, ptr.get());
}

// The following code should not pass the compilation.
// TEST(scoped_array, copy_and_assignment)
// {
//     wsd::ScopedArray<int> ptr1(new int[3]);
//     wsd::ScopedArray<int> ptr2(ptr1);

//     ptr2 = ptr1;
// }

TEST(scoped_array, exception_safety)
{
    ES_NO_THROW(wsd::ScopedArray<TestClass>());
    ES_ANY_THROW(wsd::ScopedArray<TestClass>(new TestClass[5]));

    wsd::ScopedArray<TestClass> t(new TestClass[5]);
    ES_NO_THROW(t.get());
    ES_NO_THROW(t[3]);
    ES_NO_THROW((void) !t);
    ES_NO_THROW(t.reset());
    ES_ANY_THROW(t.reset(new TestClass[5]));

    wsd::ScopedArray<TestClass> t2(new TestClass[5]);
    ES_NO_THROW(t.swap(t2));
}
