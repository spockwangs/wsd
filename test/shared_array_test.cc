// -*- coding: utf-8-unix; -*-
#include "shared_array.h"
#include "gtest/gtest.h"
#include <cstdlib>
#include "es_test.h"

using namespace wsd;

namespace {

    template <typename T>
    void fillArrayWithRandom(T *pa, size_t n)
    {
        WSD_ASSERT(pa != NULL);
        for (size_t i = 0; i < n; i++)
            pa[i] = random();
    }
    
    // Check if two arrays have the same elements.
    template <typename T>
    bool cmpEqual(const T& a, const T& b, size_t n)
    {
        for (size_t i = 0; i < n; i++)
            if (a[i] != b[i])
                return false;
        return true;
    }

}  // namespace

TEST(shared_array, constructor)
{
    SharedArray<int> ia1;
    EXPECT_EQ(1U, ia1.usecount());
    EXPECT_EQ(NULL, ia1.get());
    
    SharedArray<int> ia2(new int[3]);
    EXPECT_EQ(1U, ia2.usecount());
    EXPECT_TRUE(ia2.get() != NULL);
}


TEST(shared_array, copy)
{
    SharedArray<int> ia1(new int[10]);
    fillArrayWithRandom(ia1.get(), 10);

    SharedArray<int> ia2(ia1);
    EXPECT_TRUE(cmpEqual(ia1, ia2, 10));

    SharedArray<int> ia3;
    ia3 = ia2;
    EXPECT_TRUE(cmpEqual(ia2, ia3, 10));

    EXPECT_EQ(3U, ia1.usecount());
    EXPECT_EQ(3U, ia2.usecount());
    EXPECT_EQ(3U, ia3.usecount());
}

TEST(shared_array, swap)
{
    SharedArray<int> ia1(new int[10]);
    fillArrayWithRandom(ia1.get(), 10);
    EXPECT_EQ(1U, ia1.usecount());

    SharedArray<int> ia2(ia1);
    SharedArray<int> ia3;
    ia3.swap(ia1);
    EXPECT_EQ(NULL, ia1.get());
    EXPECT_EQ(1U, ia1.usecount());
    EXPECT_EQ(2U, ia2.usecount());
    EXPECT_EQ(2U, ia3.usecount());
    EXPECT_TRUE(cmpEqual(ia2, ia3, 10));

    swap(ia1, ia2);
    EXPECT_EQ(NULL, ia2.get());
    EXPECT_EQ(1U, ia2.usecount());
    EXPECT_EQ(2U, ia1.usecount());
    EXPECT_TRUE(cmpEqual(ia1, ia3, 10));
}

TEST(shared_array, reset)
{
    SharedArray<int> ia1(new int[10]);
    fillArrayWithRandom(ia1.get(), 10);
    EXPECT_EQ(1U, ia1.usecount());

    ia1.reset();
    EXPECT_EQ(1U, ia1.usecount());
    EXPECT_EQ(NULL, ia1.get());

    int *pa = new int[20];
    fillArrayWithRandom(pa, 20);
    ia1.reset(pa);
    EXPECT_TRUE(cmpEqual(ia1.get(), pa, 20));
}
    
void no_op_deleter(int *)
{}

TEST(shared_array, no_op_deleter)
{
    int a[10];
    SharedArray<int> ia1(a, no_op_deleter);
    ia1[3] = 0xdead;
    ia1[5] = 0xbeaf;
    ia1.reset();
    EXPECT_EQ(0xdead, a[3]);
    EXPECT_EQ(0xbeaf, a[5]);
}

struct TestDeleter {
    TestDeleter& operator=(const TestDeleter&)
    {
        this_can_throw();
        return *this;
    }

    void operator()(const TestClass *p)
    {
        delete[] p;
    }
};

TEST(shared_array, exception_safety)
{
    ES_NO_THROW(wsd::SharedArray<TestClass>());
    ES_ANY_THROW(wsd::SharedArray<TestClass>(new TestClass[5]));

    TestDeleter d;
    ES_ANY_THROW(wsd::SharedArray<TestClass>(new TestClass[5], d));

    wsd::SharedArray<TestClass> t(new TestClass[5]);
    ES_NO_THROW(wsd::SharedArray<TestClass>(t));

    wsd::SharedArray<TestClass> t2(new TestClass[10]);
    ES_NO_THROW(t = t2);

    ES_NO_THROW(t[0]);
    ES_NO_THROW(t.get());
    ES_NO_THROW(!t);
    ES_NO_THROW(t.usecount());
    ES_NO_THROW(t.unique());
    ES_NO_THROW(t2.reset());
    ES_ANY_THROW(t2.reset(new TestClass[5]));
    ES_ANY_THROW(t2.reset(new TestClass[3], d));
    ES_NO_THROW(t.swap(t2));
}
