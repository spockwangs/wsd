// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "wsd_exception.h"
#include "gtest/gtest.h"
#include <errno.h>
#include <iostream>

using namespace wsd;
using namespace std;

TEST(WsdException, constructor)
{
    // Test default constructor.
    try {
        throw WsdException();
    } catch (WsdException& e) {
        EXPECT_STREQ("WsdException thrown", e.what());

        e << "xxx";
        EXPECT_STREQ("WsdException thrown: xxx", e.what());
    }

    // Test constructor with file name and line number.
    try {
        throw WsdException("xxx.cc", 1234);
    } catch (WsdException& e) {
        EXPECT_STREQ("WsdException thrown at xxx.cc:1234", e.what());
    }
    
    // Test operator<<().
    try {
        throw WsdException() << 1 << ", xxx";
    } catch (WsdException& e) {
        EXPECT_STREQ("WsdException thrown: 1, xxx", e.what());
    }

    // Test operator<<().
    try {
        throw WsdException("xxx.cc", 1000) << "error";
    } catch (WsdException& e) {
        EXPECT_STREQ("WsdException thrown at xxx.cc:1000: error", e.what());
    }
}

TEST(WsdException, copy)
{
    try {
        throw WsdException();
    } catch (WsdException& e) {
        EXPECT_STREQ("WsdException thrown", e.what());

        WsdException e2(e);
        EXPECT_STREQ("WsdException thrown", e2.what());

        e << "xxx";
        e2 << "yyy";
        EXPECT_STREQ("WsdException thrown: xxx", e.what());
        EXPECT_STREQ("WsdException thrown: yyy", e2.what());
    }
}

TEST(WsdException, SyscallException)
{
    try {
        throw SyscallException("xxx", 123, EBUSY);
    } catch (SyscallException& e) {
        EXPECT_EQ(EBUSY, e.error());
        EXPECT_STREQ("SyscallException thrown at xxx:123: Device or resource busy", e.what());
    }

    try {
        throw SyscallException("xxx", 123, EBUSY) << "invalid arguments";
    } catch (SyscallException& e) {
        EXPECT_EQ(EBUSY, e.error());
        EXPECT_STREQ("SyscallException thrown at xxx:123: Device or resource busy;"
                     " invalid arguments", e.what());
    }
}
