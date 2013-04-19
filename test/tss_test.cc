// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "tss.h"
#include "gtest/gtest.h"
#include <iostream>
#include "es_test.h"

using namespace std;

TEST(Tss, constructor)
{
    wsd::ThreadSpecificPtr<int> tss_int;
    int *p = new int();
    tss_int.reset(p);
    EXPECT_EQ(p, tss_int.get());
    EXPECT_EQ(0, *tss_int);
}

TEST(Tss, cleanup)
{
    wsd::ThreadSpecificPtr<int> tss_int(NULL);
    int a;
    tss_int.reset(&a);
    *tss_int = 3;
    EXPECT_EQ(3, *tss_int);
}

void cleanup(void *p)
{
    delete static_cast<int*>(p);
}

TEST(Tss, custom_cleanup)
{
    wsd::ThreadSpecificPtr<int> tss_int(cleanup);
    int *p = new int();
    tss_int.reset(p);
    EXPECT_EQ(0, *tss_int);
    *tss_int = 0xdead;
    EXPECT_EQ(0xdead, *tss_int);
}

void cleanup_test_class(void *p)
{
    delete static_cast<TestClass*>(p);
}

TEST(Tss, exception_safety)
{
    ES_MAY_THROW(wsd::ThreadSpecificPtr<TestClass>(), wsd::SyscallException);
    ES_MAY_THROW(wsd::ThreadSpecificPtr<TestClass>(NULL), wsd::SyscallException);
    ES_MAY_THROW(wsd::ThreadSpecificPtr<TestClass>(cleanup_test_class), wsd::SyscallException);

    wsd::ThreadSpecificPtr<TestClass> t;
    ES_NO_THROW(t.get());
    ES_NO_THROW(t.operator->());
    ES_NO_THROW(*t);
    ES_NO_THROW(t.release());

    t.reset(new TestClass());
    ES_NO_THROW(t.get());
    ES_NO_THROW(t.operator->());
    ES_NO_THROW(*t);

    TestClass *p = t.get();
    ES_NO_THROW(p = t.release());
    delete p;
    
    ES_ANY_THROW(t.reset(new TestClass()));
}
