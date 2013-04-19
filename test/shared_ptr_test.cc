// -*- coding: utf-8-unix; -*-
#include "shared_ptr.h"
#include "enable_shared_from_this.h"
#include "gtest/gtest.h"
#include "es_test.h"
#include <iostream>

using namespace wsd;
using namespace std;

TEST(shared_ptr, constructor)
{
    SharedPtr<int> ptr;
    EXPECT_EQ(NULL, ptr.get());
    EXPECT_FALSE(ptr);

    int *pi = new int();
    SharedPtr<int> ptr2(pi);
    EXPECT_EQ(pi, ptr2.get());
    EXPECT_EQ(1U, ptr2.usecount());
    EXPECT_EQ(*pi, *ptr2);

    SharedPtr<int> ptr3(ptr2);
    EXPECT_EQ(pi, ptr3.get());
    EXPECT_EQ(*pi, *ptr3);
    EXPECT_EQ(2U, ptr3.usecount());
    EXPECT_EQ(2U, ptr2.usecount());
}

TEST(shared_ptr, assign)
{
    SharedPtr<int> ptr(new int(0xbeaf));
    SharedPtr<int> ptr2;
    ptr2 = ptr;
    EXPECT_EQ(*ptr, *ptr2);
    EXPECT_EQ(2U, ptr.usecount());
    EXPECT_EQ(2U, ptr2.usecount());
    EXPECT_EQ(ptr.get(), ptr2.get());
}

TEST(shared_ptr, swap)
{
    SharedPtr<int> ptr1(new int(0xdead));
    SharedPtr<int> ptr2(new int(0xbeaf));
    EXPECT_EQ(0xdead, *ptr1);
    EXPECT_EQ(0xbeaf, *ptr2);

    swap(ptr1, ptr2);
    EXPECT_EQ(0xbeaf, *ptr1);
    EXPECT_EQ(0xdead, *ptr2);
}

class X : public EnableSharedFromThis<X> {
public:

    int getX() { return 0xdead; }
};

TEST(shared_ptr, enable_shared_from_this)
{
    SharedPtr<X> px(new X);
    SharedPtr<X> px2 = px->sharedFromThis();
    EXPECT_EQ(2U, px.usecount());
    EXPECT_EQ(0xdead, px->getX());
    EXPECT_EQ(0xdead, px2->getX());
    EXPECT_EQ(px, px2);
    EXPECT_TRUE(!(px < px2 || px > px2));

    X x;
    SharedPtr<X> px3 = x.sharedFromThis();
    EXPECT_FALSE(px3);
}

// Verify that SharedPtr<T>'s destructor does not require the completeness of T.
class Y;
TEST(shared_ptr, completeness)
{
    SharedPtr<Y> y;
}

// no-op deallocator
void no_op(void *)
{
}

TEST(shared_ptr, with_a_deleter)
{
    int i;
    SharedPtr<int> pi(&i, &no_op);
    *pi = 0xdead;
    pi.reset();
    EXPECT_EQ(0xdead, i);
}

struct TestDeleter {
    TestDeleter() { this_can_throw(); }

    TestDeleter& operator=(const TestDeleter&)
    {
        this_can_throw();
        return *this;
    }
    
    void operator()(const TestClass*p)
    {
        delete p;
    }
};
    
TEST(shared_ptr, exception_safety)
{
    ES_NO_THROW(wsd::SharedPtr<TestClass>());

    ES_ANY_THROW(wsd::SharedPtr<TestClass>(new TestClass(0xDD)));
    TestDeleter d;
    ES_ANY_THROW(wsd::SharedPtr<TestClass>(new TestClass(0xEE), d));

    // Copy constructor 
    ES_NO_THROW(wsd::SharedPtr<TestClass>(wsd::SharedPtr<TestClass>()));

    wsd::SharedPtr<TestClass> t(new TestClass());
    ES_NO_THROW(wsd::SharedPtr<TestClass>(t));
    
    // Assignment operator should not throw.
    wsd::SharedPtr<TestClass> t1(new TestClass(0xAA)), t2(new TestClass(0xBB));
    ES_NO_THROW(t1 = t2);

    // operator->() should not throw.
    ES_NO_THROW(t1.operator->());

    // operator*() should not throw.
    ES_NO_THROW(*t1);

    // get() should not throw
    ES_NO_THROW(t1.get());

    // operator unspecified_bool_type() should not throw
    ES_NO_THROW(t1.operator unspecified_bool_type());

    // unique() should not throw
    ES_NO_THROW(t1.unique());

    // usecount() should not throw
    ES_NO_THROW(t1.usecount());
    
    // reset()
    ES_ANY_THROW(t1.reset(new TestClass(0xCC)));
    ES_ANY_THROW(t1.reset(new TestClass(0xC0), d));

    // swap() should not throw
    ES_NO_THROW(t1.swap(t2));
}
