// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "exception_ptr.h"
#include "gtest/gtest.h"
#include <stdexcept>
#include "es_test.h"

TEST(exception_ptr, current_exception_std_exception)
{
    try {
        throw std::invalid_argument("");
    } catch (...) {
        ES_NO_THROW(wsd::currentException());
        EXPECT_THROW(wsd::currentException()->rethrow(), std::logic_error);
    }
}

TEST(exception_ptr, current_exception_wsd_exception)
{
    try {
        throw wsd::WsdException();
    } catch (...) {
        ES_NO_THROW(wsd::currentException());
        EXPECT_THROW(wsd::currentException()->rethrow(), wsd::WsdException);
    }

    try {
        wsd::throwException(wsd::SyscallException(__FILE__, __LINE__, 1));
    } catch (...) {
        ES_NO_THROW(wsd::currentException());
        EXPECT_THROW(wsd::currentException()->rethrow(), wsd::SyscallException);
    }
}

TEST(exception_ptr, current_exception_unknown_exception)
{
    try {
        throw "unkown";
    } catch (...) {
        ES_NO_THROW(wsd::currentException());
        EXPECT_THROW(wsd::currentException()->rethrow(), wsd::CurrentExceptionUnknownException);
    }
}

TEST(exception_ptr, current_exception_bad_alloc)
{
    try {
        new char[0xFFFFFFFFFFFF];
        FAIL() << "should not reach here";
    } catch (...) {
        ES_NO_THROW(wsd::currentException());
        EXPECT_THROW(wsd::currentException()->rethrow(), std::bad_alloc);
    }
}

TEST(exception_ptr, enable_current_exception)
{
    ES_ANY_THROW(wsd::enableCurrentException(std::bad_alloc()));
    ES_ANY_THROW(wsd::enableCurrentException(wsd::WsdException()));
    ES_ANY_THROW(wsd::enableCurrentException(wsd::SyscallException(__FILE__, __LINE__, 0)));
}

TEST(exception_ptr, copy_exception)
{
    ES_NO_THROW(wsd::copyException(std::bad_alloc()));
    ES_NO_THROW(wsd::copyException(wsd::WsdException()));
    ES_NO_THROW(wsd::copyException(wsd::SyscallException(__FILE__, __LINE__, 0)));
}
