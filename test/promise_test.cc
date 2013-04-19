// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "promise.h"
#include "gtest/gtest.h"
#include "bind.h"
#include "exception_ptr.h"
#include <iostream>
#include "when_all.h"
#include "es_test.h"

using namespace std;

bool g_has_run = false;

TEST(promise, constructor)
{
    wsd::Promise<int> promise;
    wsd::Future<int> future = promise.getFuture();
    EXPECT_FALSE(future.isDone());
    EXPECT_FALSE(future.hasValue());
}

void do_value(const wsd::Future<int>& i)
{
    EXPECT_EQ(10, i.get());
}

TEST(promise, do_value)
{
    wsd::Promise<int> promise;
    wsd::Future<int> future = promise.getFuture();
    
    future.then(wsd::bind(&do_value));
    promise.setValue(10);
    EXPECT_TRUE(future.hasValue());
    EXPECT_EQ(10, future.get());
    EXPECT_THROW(promise.setValue(1), wsd::PromiseAlreadySatisfiedException);
}

void do_exception(const wsd::Future<int>& i)
{
    EXPECT_THROW(i.get(), wsd::CurrentExceptionUnknownException);
}

TEST(promise, do_exception)
{
    wsd::Promise<int> promise;
    wsd::Future<int> future = promise.getFuture();
    
    future.then(wsd::bind(&do_exception));
    try {
        throw "error";
    } catch (...) {
        promise.setException(wsd::currentException());
    }
    EXPECT_TRUE(future.isDone());
    EXPECT_TRUE(future.hasException());
    EXPECT_THROW(future.get(), wsd::CurrentExceptionUnknownException);
    EXPECT_THROW(promise.setValue(1), wsd::PromiseAlreadySatisfiedException);
}

wsd::Future<int> async_get_int(const wsd::Future<int>& f)
{
    int i = f.get();
    wsd::Promise<int> p;
    p.setValue(i+1);
    return p.getFuture();
}

void do_async_int(const wsd::Future<int>& f)
{
    try {
        EXPECT_EQ(9, f.get());
    } catch (...) {
        FAIL();
    }
}

wsd::Future<int> async_get_exception(const wsd::Future<int>&)
{
    throw 0;
}

void do_async_exception(const wsd::Future<int>& f)
{
    try {
        f.get();
        FAIL();
    } catch (int& e) {
        EXPECT_EQ(0, e);
    }
}

TEST(promise, then_then)
{
    // Test sequential composition of two futures.
    wsd::Promise<int> promise;
    wsd::Future<int> future = promise.getFuture();
    future.then(wsd::bind(&async_get_int))
          .then(wsd::bind(&do_async_int));
    promise.setValue(8);
}

TEST(promise, then_then2)
{
    // Test sequential composition of two futures with the first throwing an exception.
    wsd::Promise<int> promise;
    wsd::Future<int> future = promise.getFuture();
    future.then(wsd::bind(&async_get_exception))
          .then(wsd::bind(&do_async_exception));
    promise.setValue(8);
}
    
wsd::Future<void> async_get_void(const wsd::Future<int>&)
{
    wsd::Promise<void> p;
    p.set();
    return p.getFuture();
}

void do_async_void(const wsd::Future<void>& f)
{
    EXPECT_NO_THROW(f.get());
}

TEST(promise, then_then3)
{
    wsd::Promise<int> promise;
    wsd::Future<int> future = promise.getFuture();
    future.then(wsd::bind(&async_get_void))
          .then(wsd::bind(&do_async_void));
    promise.setValue(8);
}

wsd::Future<int> async_inc(const wsd::Future<int>& f)
{
    wsd::Promise<int> p;
    p.setValue(f.get()+1);
    return p.getFuture();
}

wsd::Future<void> async_process(const wsd::Future<int>& f)
{
    wsd::Promise<void> p;
    EXPECT_NO_THROW({
        int i = f.get();
        (void) i;
        p.set();
                    });
    return p.getFuture();
}

void complete(const wsd::Future<void>& f)
{
    EXPECT_NO_THROW(f.get());
}

TEST(promise, then_then4)
{
    // Test sequential composition of three futures: the first returns Future<int>, and
    // the second returns Future<void>.
    wsd::Promise<int> promise;
    wsd::Future<int> future = promise.getFuture();
    future.then(wsd::bind(&async_inc))
          .then(wsd::bind(&async_process))
          .then(wsd::bind(&complete));
}

wsd::Future<int> async_exception(const wsd::Future<int>&)
{
    throw 0;
}

wsd::Future<void> async_process2(const wsd::Future<int>& f)
{
    int i = f.get();
    (void) i;
    return wsd::makeFuture();
}

void complete2(const wsd::Future<void>& f)
{
    EXPECT_THROW(f.get(), int);
}

TEST(promise, then_then5)
{
    // Test sequential composition of three futures: the first returns Futures<int> with an exception, and
    // the second returns Future<void>.
    wsd::Promise<int> promise;
    wsd::Future<int> future = promise.getFuture();
    future.then(wsd::bind(&async_exception))
          .then(wsd::bind(&async_process2))
          .then(wsd::bind(&complete2));
}

wsd::Future<void> async_process3(const wsd::Future<int>& f)
{
    try {
        f.get();
        ADD_FAILURE();
    } catch (int& e) {
        EXPECT_EQ(0, e);
        throw "xxx";
    }
    return wsd::makeFuture();    
}

void complete3(const wsd::Future<void>& f)
{
    EXPECT_THROW(f.get(), const char*);
}

TEST(promise, then_then6)
{
    // Test sequential composition of three futures: the first returns Future<int> with an exception, and
    // the second captures the exception and returns Future<void> with another exception.
    wsd::Promise<int> promise;
    wsd::Future<int> future = promise.getFuture();
    future.then(wsd::bind(&async_exception))
          .then(wsd::bind(&async_process3))
          .then(wsd::bind(&complete3));
}

void getThree(const wsd::Future<int>& f)
{
    EXPECT_EQ(3, f.get());
}

TEST(promise, prompt_future)
{
    wsd::makeFuture<int>(3).then(wsd::bind(&getThree));
}

void getVoid(const wsd::Future<void>& f)
{
    EXPECT_NO_THROW(f.get());
    g_has_run = true;
}

void getException(const wsd::Future<int>& f)
{
    EXPECT_THROW(f.get(), wsd::WsdException);
    g_has_run = true;
}

TEST(promise, prompt_exception)
{
    g_has_run = false;
    wsd::Future<int>(wsd::copyException(wsd::WsdException())).then(wsd::bind(&getException));
    EXPECT_TRUE(g_has_run);
}

TEST(promise, prompt_future2)
{
    g_has_run = false;
    wsd::makeFuture().then(wsd::bind(&getVoid));
    EXPECT_TRUE(g_has_run);
}

void do_all_value(const wsd::Future<wsd::Tuple<wsd::Future<int>, wsd::Future<int> > >& p)
{
    const wsd::Tuple<wsd::Future<int>, wsd::Future<int> >& t = p.get();
    EXPECT_EQ(3, t.get<0>().get());
    EXPECT_EQ(4, t.get<1>().get());
}

TEST(promise, when_all)
{
    wsd::Promise<int> p1;
    wsd::Promise<int> p2;

    wsd::whenAll(p1.getFuture(), p2.getFuture()).then(wsd::bind(&do_all_value));

    p1.setValue(3);
    sleep(1);
    p2.setValue(4);
}

void do_value_exception(const wsd::Future<wsd::Tuple<wsd::Future<int>, wsd::Future<int> > >& p)
{
    const wsd::Tuple<wsd::Future<int>, wsd::Future<int> >& t = p.get();
    EXPECT_EQ(3, t.get<0>().get());
    EXPECT_THROW(t.get<1>().get(), wsd::WsdException);
}

TEST(promise, when_all_with_one_exception)
{
    wsd::Promise<int> p1;
    wsd::Promise<int> p2;

    wsd::whenAll(p1.getFuture(), p2.getFuture()).then(wsd::bind(&do_value_exception));

    p1.setValue(3);
    sleep(1);

    p2.setException(wsd::copyException(wsd::WsdException()));
}

void do_two_exceptions(const wsd::Future<wsd::Tuple<wsd::Future<int>, wsd::Future<int> > >& p)
{
    const wsd::Tuple<wsd::Future<int>, wsd::Future<int> >& t = p.get();
    EXPECT_THROW(t.get<0>().get(), wsd::WsdException);
    EXPECT_THROW(t.get<1>().get(), wsd::WsdException);
}

TEST(promise, when_all_with_two_exceptions)
{
    wsd::Promise<int> p1;
    wsd::Promise<int> p2;

    wsd::whenAll(p1.getFuture(), p2.getFuture()).then(wsd::bind(&do_two_exceptions));

    p1.setException(wsd::copyException(wsd::WsdException()));
    sleep(1);
    p2.setException(wsd::copyException(wsd::WsdException()));
}

void do_three_values(const wsd::Future<wsd::Tuple<wsd::Future<bool>, wsd::Future<int>, wsd::Future<string> > >& p)
{
    const wsd::Tuple<wsd::Future<bool>, wsd::Future<int>, wsd::Future<string> >& t = p.get();
    EXPECT_EQ(true, t.get<0>().get());
    EXPECT_EQ(34, t.get<1>().get());
    EXPECT_EQ("hello", t.get<2>().get());
}

TEST(promise, when_all_with_three_values)
{
    wsd::Promise<bool> p1;
    wsd::Promise<int> p2;
    wsd::Promise<string> p3;

    try {
        wsd::whenAll(p1.getFuture(), p2.getFuture(), p3.getFuture()).then(wsd::bind(&do_three_values));

        p1.setValue(true);
        p2.setValue(34);
        sleep(1);
        p3.setValue("hello");
    } catch (...) {}
}

void do_two_values_and_one_exception(
        const wsd::Future<wsd::Tuple<wsd::Future<bool>, wsd::Future<int>, wsd::Future<string> > >& p)
{
    const wsd::Tuple<wsd::Future<bool>, wsd::Future<int>, wsd::Future<string> >& t = p.get();
    EXPECT_EQ(true, t.get<0>().get());
    EXPECT_THROW(t.get<1>().get(), wsd::WsdException);
    EXPECT_EQ("hello", t.get<2>().get());
}

TEST(promise, when_all_with_two_values_and_one_exception)
{
    try {
        wsd::Promise<bool> p1;
        wsd::Promise<int> p2;
        wsd::Promise<string> p3;

        wsd::whenAll(p1.getFuture(), p2.getFuture(), p3.getFuture())
            .then(wsd::bind(&do_two_values_and_one_exception));

        p1.setValue(true);
        p2.setException(wsd::copyException(wsd::WsdException()));
        sleep(1);
        p3.setValue("hello");
    } catch (...) {}
}

int do_bool(const wsd::Future<bool>& future)
{
    EXPECT_EQ(true, future.get());
    return 41;
}

long do_int(const wsd::Future<int>& future)
{
    EXPECT_EQ(41, future.get());
    return 0xdead;
}

void do_long(const wsd::Future<long>& future)
{
    EXPECT_EQ(0xdead, future.get());
    wsd::throwException(wsd::WsdException());
}

void do_exception2(const wsd::Future<void>& future)
{
    EXPECT_THROW(future.get(), wsd::WsdException);
}

TEST(promise, then)
{
    wsd::Promise<bool> p1;

    p1.getFuture()
      .then(wsd::bind(&do_bool))
      .then(wsd::bind(&do_int))
      .then(wsd::bind(&do_long))
      .then(wsd::bind(&do_exception2));
    p1.setValue(true);
}

void do_vector_futures(const wsd::Future<std::vector<wsd::Future<int> > >& future)
{
    const std::vector<wsd::Future<int> >& futures = future.get();
    for (size_t i = 0; i < futures.size(); i++)
        EXPECT_EQ(i, (size_t) futures[i].get());
}

TEST(promise, iterator)
{
    try {
        std::vector<wsd::Promise<int> > promises;
        for (size_t i = 0; i < 100; i++)
            promises.push_back(wsd::Promise<int>());
        std::vector<wsd::Future<int> > futures;
        for (size_t i = 0; i < promises.size(); i++)
            futures.push_back(promises[i].getFuture());

        wsd::whenAll<int>(futures.begin(), futures.end())
            .then(wsd::bind(&do_vector_futures));
        for (size_t i = 0; i < promises.size(); i++) {
            promises[i].setValue(i);
            EXPECT_EQ(i, (size_t) futures[i].get());
        }
        for (size_t i = 0; i < futures.size(); i++)
            EXPECT_EQ(i, (size_t) futures[i].get());
    } catch (...) {}
}

void run_or_not_run(const wsd::Future<TestClass>& f)
{
    EXPECT_TRUE(f.isDone());
    g_has_run = true;
}

void run_or_not_run_void(const wsd::Future<void>& f)
{
    EXPECT_TRUE(f.isDone());
    g_has_run = true;
}

TEST(promise, exception_safety)
{
    ES_MAY_THROW(wsd::Promise<void>(), std::bad_alloc);
    wsd::Promise<void> p;
    ES_NO_THROW(p.getFuture());
    ES_NO_THROW(p.getFuture().isDone());
    p.set();
    ES_NO_THROW(p.getFuture().isDone());
    ES_NO_THROW(p.getFuture().get());
    
    // Promise<void>::setException() should not throw exception.
    wsd::Promise<void> p2;
    ES_NO_THROW(p2.setException(wsd::copyException(wsd::WsdException())));
    ES_NO_THROW(p2.getFuture().isDone());
    EXPECT_TRUE(p2.getFuture().isDone());
    EXPECT_ANY_THROW(p2.getFuture().get());

    ES_MAY_THROW(wsd::Promise<TestClass>(), std::bad_alloc);
    wsd::Promise<TestClass> p3;
    ES_NO_THROW(p3.getFuture());
    ES_NO_THROW(p3.getFuture().isDone());
    p3.setValue(TestClass());
    ES_NO_THROW(p3.getFuture().isDone());
    ES_MAY_THROW2(p3.getFuture().get(), int, std::bad_alloc);

    wsd::Promise<TestClass> p4;
    ES_NO_THROW(p4.setException(wsd::copyException(wsd::WsdException())));
    ES_NO_THROW(p4.getFuture().isDone());
    EXPECT_ANY_THROW(p4.getFuture().get());
    
    // Test the exception-safety of then().  If then() throws an exception the registered
    // callback should not run, and if not the callback should run once the future is
    // satisfied.

    // set() before then().
    bool succeeded = false;
    for (int next_throw_count = 0; !succeeded; next_throw_count++) {
        g_throw_counter = next_throw_count;
        g_has_run = false;
        try {
            wsd::Promise<TestClass> p;
            p.setValue(TestClass());
            wsd::Future<TestClass> f = p.getFuture();
            f.then(wsd::bind(&run_or_not_run));
            succeeded = true;
            EXPECT_TRUE(g_has_run);
        } catch (...) {
            EXPECT_FALSE(g_has_run);
        }
    }
    g_throw_counter = -1;
    
    // set() after then()
    succeeded = false;
    for (int next_throw_count = 0; !succeeded; next_throw_count++) {
        g_throw_counter = next_throw_count;
        g_has_run = false;
        try {
            wsd::Promise<TestClass> p;
            wsd::Future<TestClass> f = p.getFuture();
            f.then(wsd::bind(&run_or_not_run));
            p.setValue(TestClass());
            succeeded = true;
            EXPECT_TRUE(g_has_run);
        } catch (...) {
            EXPECT_FALSE(g_has_run);
        }
    }
    g_throw_counter = -1;

    // setException() before then().
    succeeded = false;
    for (int next_throw_count = 0; !succeeded; next_throw_count++) {
        g_throw_counter = next_throw_count;
        g_has_run = false;
        try {
            wsd::Promise<TestClass> p;
            p.setException(wsd::copyException(wsd::WsdException()));
            wsd::Future<TestClass> f = p.getFuture();
            f.then(wsd::bind(&run_or_not_run));
            succeeded = true;
            EXPECT_TRUE(g_has_run);
        } catch (std::exception& e) {
            EXPECT_FALSE(g_has_run);
        }
    }
    g_throw_counter = -1;
    
    // setException() after then().
    succeeded = false;
    for (int next_throw_count = 0; !succeeded; next_throw_count++) {
        g_throw_counter = next_throw_count;
        g_has_run = false;
        try {
            wsd::Promise<TestClass> p;
            wsd::Future<TestClass> f = p.getFuture();
            f.then(wsd::bind(&run_or_not_run));
            p.setException(wsd::copyException(wsd::WsdException()));
            succeeded = true;
            EXPECT_TRUE(g_has_run);
        } catch (std::exception& e) {
            EXPECT_FALSE(g_has_run);
        }
    }
    g_throw_counter = -1;

    // Do the same test for Promise<void>.
    //
    // set() before then()
    succeeded = false;
    for (int next_throw_count = 0; !succeeded; next_throw_count++) {
        g_throw_counter = next_throw_count;
        g_has_run = false;
        try {
            wsd::Promise<void> p;
            p.set();
            wsd::Future<void> f = p.getFuture();
            f.then(wsd::bind(&run_or_not_run_void));
            succeeded = true;
            EXPECT_TRUE(g_has_run);
        } catch (...) {
            EXPECT_FALSE(g_has_run);
        }
    }
    g_throw_counter = -1;
    
    // set() after then().
    succeeded = false;
    for (int next_throw_count = 0; !succeeded; next_throw_count++) {
        g_throw_counter = next_throw_count;
        g_has_run = false;
        try {
            wsd::Promise<void> p;
            wsd::Future<void> f = p.getFuture();
            f.then(wsd::bind(&run_or_not_run_void));
            p.set();
            succeeded = true;
            EXPECT_TRUE(g_has_run);
        } catch (...) {
            EXPECT_FALSE(g_has_run);
        }
    }
    g_throw_counter = -1;
    
    // setException() before then().
    succeeded = false;
    for (int next_throw_count = 0; !succeeded; next_throw_count++) {
        g_throw_counter = next_throw_count;
        g_has_run = false;
        try {
            wsd::Promise<void> p;
            p.setException(wsd::copyException(wsd::WsdException()));
            wsd::Future<void> f = p.getFuture();
            f.then(wsd::bind(&run_or_not_run_void));
            succeeded = true;
            EXPECT_TRUE(g_has_run);
        } catch (...) {
            EXPECT_FALSE(g_has_run);
        }
    }

    // setException() after then().
    succeeded = false;
    for (int next_throw_count = 0; !succeeded; next_throw_count++) {
        g_throw_counter = next_throw_count;
        g_has_run = false;
        try {
            wsd::Promise<void> p;
            wsd::Future<void> f = p.getFuture();
            f.then(wsd::bind(&run_or_not_run_void));
            p.setException(wsd::copyException(wsd::WsdException()));
            succeeded = true;
            EXPECT_TRUE(g_has_run);
        } catch (...) {
            EXPECT_FALSE(g_has_run);
        }
    }
}
