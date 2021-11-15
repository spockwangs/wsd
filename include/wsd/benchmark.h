// Copyright (c) 2021 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include "wsd/detail/benchmark_detail.h"
#include <string>
#include <functional>
#include <initializer_list>
#include <cassert>
#include <atomic>
#include <thread>
#include <unordered_map>

namespace wsd {
namespace benchmark {

void Init(int* argc, char*** argv);

int RunTests();

class Test {
public:
    Test()
    {}

    virtual ~Test()
    {}

    virtual void SetUp()
    {}

    virtual void TearDown()
    {}

    virtual int TestBody() = 0;
};

}  // namespace benchmark
}  // namespace wsd

#define MAKE_CLASS_NAME_(class_name, method_name) class_name ## method_name

#define TEST_CASE(class_name, method_name)                              \
    class MAKE_CLASS_NAME_(class_name, method_name) : public class_name { \
    public:                                                             \
        virtual int TestBody();                                         \
    private:                                                            \
        static void* s_dummy;                                           \
    };                                                                  \
    void* MAKE_CLASS_NAME_(class_name, method_name)                     \
        ::s_dummy = ::wsd::benchmark::detail::BenchmarkManager::GetInstance().Register( \
                #class_name, #method_name, new ::wsd::benchmark::detail::TestFactoryImpl<MAKE_CLASS_NAME_(class_name, method_name)>()); \
    int MAKE_CLASS_NAME_(class_name, method_name)::TestBody()
