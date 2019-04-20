// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "when_all.h"
#include "gtest/gtest.h"
#include "es_test.h"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <tuple>

using namespace std;

bool g_has_run = false;
bool g_has_exception = false;

void run_or_not_run(
        const wsd::Future<std::tuple<wsd::Future<TestClass>, wsd::Future<TestClass>, wsd::Future<TestClass> > >& future)
{
    g_has_run = true;
    g_has_exception = false;
    try {
        EXPECT_TRUE(future.isDone());
        EXPECT_EQ(0xAA, *std::get<0>(future.get()).get().p);
        EXPECT_EQ(0xBB, *std::get<1>(future.get()).get().p);
        EXPECT_EQ(0xCC, *std::get<2>(future.get()).get().p);
    } catch (...) {
        g_has_exception = true;
    }
}

void run_or_not_run_vector(const wsd::Future<vector<wsd::Future<TestClass> > >& future)
{
    g_has_run = true;
    g_has_exception = false;
    try {
        EXPECT_TRUE(future.isDone());
        EXPECT_EQ(0xAA, *future.get()[0].get().p);
        EXPECT_EQ(0xBB, *future.get()[1].get().p);
        EXPECT_EQ(0xCC, *future.get()[2].get().p);
    } catch (...) {
        g_has_exception = true;
    }
}

TEST(when_all, exception_safety)
{
    // If an exception is thrown by whenAll(), the registered callback should not
    // run, but if not the registered callback should run once the future is satisfied.

    // set() after then().
    bool succeeded = false;
    for (int next_throw_count = 0; !succeeded; next_throw_count++) {
        g_throw_counter = next_throw_count;
        g_has_run = false;
        try {
            wsd::Promise<TestClass> p1, p2, p3;
            wsd::whenAll(p1.getFuture(), p2.getFuture(), p3.getFuture())
                .then(wsd::bind(&run_or_not_run));
            p1.setValue(TestClass(0xAA));
            p2.setValue(TestClass(0xBB));
            p3.setValue(TestClass(0xCC));
            EXPECT_TRUE(g_has_run);
            if (!g_has_exception)
                succeeded = true;
        } catch (...) {
            EXPECT_FALSE(g_has_run);
        }
    }
    g_throw_counter = -1;

    // set() before then().
    succeeded = false;
    for (int next_throw_count = 0; !succeeded; next_throw_count++) {
        g_throw_counter = next_throw_count;
        g_has_run = false;
        try {
            wsd::Promise<TestClass> p1, p2, p3;
            p1.setValue(TestClass(0xAA));
            p2.setValue(TestClass(0xBB));
            p3.setValue(TestClass(0xCC));
            wsd::whenAll(p1.getFuture(), p2.getFuture(), p3.getFuture())
                .then(wsd::bind(&run_or_not_run));
            EXPECT_TRUE(g_has_run);
            if (!g_has_exception)
                succeeded = true;
        } catch (...) {
            EXPECT_FALSE(g_has_run);
        }
    }
    g_throw_counter = -1;

    // some set() before and some after then().
    succeeded = false;
    for (int next_throw_count = 0; !succeeded; next_throw_count++) {
        g_throw_counter = next_throw_count;
        g_has_run = false;
        try {
            wsd::Promise<TestClass> p1, p2, p3;
            p2.setValue(TestClass(0xBB));
            p3.setValue(TestClass(0xCC));

            wsd::whenAll(p1.getFuture(), p2.getFuture(), p3.getFuture())
                .then(wsd::bind(&run_or_not_run));
            p1.setValue(TestClass(0xAA));

            EXPECT_TRUE(g_has_run);
            if (!g_has_exception)
                succeeded = true;
        } catch (...) {
            EXPECT_FALSE(g_has_run);
        }
    }
    g_throw_counter = -1;

    // Do the same test for whenAll() of iterator form.
    //
    // set() after then()
    succeeded = false;
    for (int next_throw_count = 0; !succeeded; next_throw_count++) {
        g_throw_counter = next_throw_count;
        g_has_run = false;
        try {
            vector<wsd::Promise<TestClass> > ps;
            vector<wsd::Future<TestClass> > fs;
            for (int i = 0; i < 3; i++) {
                ps.push_back(wsd::Promise<TestClass>());
                fs.push_back(ps.back().getFuture());
            }
            
            wsd::whenAll<TestClass>(fs.begin(), fs.end()).then(wsd::bind(&run_or_not_run_vector));
            ps[0].setValue(TestClass(0xAA));
            ps[1].setValue(TestClass(0xBB));
            ps[2].setValue(TestClass(0xCC));
            EXPECT_TRUE(g_has_run);
            if (!g_has_exception)
                succeeded = true;
        } catch (...) {
            EXPECT_FALSE(g_has_run);
        }
    }
    g_throw_counter = -1;

    // set() before then().
    succeeded = false;
    for (int next_throw_count = 0; !succeeded; next_throw_count++) {
        g_throw_counter = next_throw_count;
        g_has_run = false;
        try {
            vector<wsd::Promise<TestClass> > ps;
            vector<wsd::Future<TestClass> > fs;
            for (int i = 0; i < 3; i++) {
                ps.push_back(wsd::Promise<TestClass>());
                fs.push_back(ps.back().getFuture());
            }
            ps[0].setValue(TestClass(0xAA));
            ps[1].setValue(TestClass(0xBB));
            ps[2].setValue(TestClass(0xCC));
            
            wsd::whenAll<TestClass>(fs.begin(), fs.end()).then(wsd::bind(&run_or_not_run_vector));
            EXPECT_TRUE(g_has_run);
            if (!g_has_exception)
                succeeded = true;
        } catch (...) {
            EXPECT_FALSE(g_has_run);
        }
    }
    g_throw_counter = -1;
    
    // some set() before and some after then().
    succeeded = false;
    for (int next_throw_count = 0; !succeeded; next_throw_count++) {
        g_throw_counter = next_throw_count;
        g_has_run = false;
        try {
            vector<wsd::Promise<TestClass> > ps;
            vector<wsd::Future<TestClass> > fs;
            for (int i = 0; i < 3; i++) {
                ps.push_back(wsd::Promise<TestClass>());
                fs.push_back(ps.back().getFuture());
            }
            ps[1].setValue(TestClass(0xBB));

            wsd::whenAll<TestClass>(fs.begin(), fs.end()).then(wsd::bind(&run_or_not_run_vector));
            ps[0].setValue(TestClass(0xAA));
            ps[2].setValue(TestClass(0xCC));
            EXPECT_TRUE(g_has_run);
            if (!g_has_exception)
                succeeded = true;
        } catch (...) {
            EXPECT_FALSE(g_has_run);
        }
    }
    g_throw_counter = -1;
}
