#include "scoped_ptr.h"
#include "gtest/gtest.h"
#include "es_test.h"

TEST(scoped_ptr, constructor)
{
    wsd::ScopedPtr<int> ptr;
    EXPECT_TRUE(ptr.get() == NULL);
    EXPECT_FALSE(ptr);

    int *pi = new int(0xdead);
    wsd::ScopedPtr<int> ptr2(pi);
    ASSERT_EQ(pi, ptr2.get());
    EXPECT_EQ(*pi, *ptr2);
    EXPECT_TRUE(ptr2);
}

TEST(scoped_ptr, reset)
{
    wsd::ScopedPtr<int> ptr(new int(0xdead));
    ASSERT_TRUE(ptr);
    EXPECT_EQ(0xdead, *ptr);

    ptr.reset();
    EXPECT_FALSE(ptr);
    EXPECT_TRUE(ptr.get() == NULL);

    int *pi = new int(0xbeaf);
    ptr.reset(pi);
    ASSERT_TRUE(ptr);
    ASSERT_EQ(pi, ptr.get());
    EXPECT_EQ(*pi, *ptr);
}

struct A {
    int foo() { return 0xdaed; }
};

TEST(scoped_ptr, operator_overload)
{
    wsd::ScopedPtr<A> ptr(new A());
    ASSERT_TRUE(ptr);
    EXPECT_EQ(0xdaed, ptr->foo());
}

// The following code should not pass the compilation.
// TEST(scoped_ptr, copy_and_assignment)
// {
//     wsd::ScopedPtr<int> ptr1(new int());
//     wsd::ScopedPtr<int> ptr2(ptr1);

//     ptr2 = ptr1;
// }

TEST(scoped_ptr, exception_safety)
{
    ES_NO_THROW(wsd::ScopedPtr<TestClass>());
    ES_ANY_THROW(wsd::ScopedPtr<TestClass>(new TestClass(0xAA)));

    wsd::ScopedPtr<TestClass> t(new TestClass(0xBB));
    ES_NO_THROW(*t);
    ES_NO_THROW(t.operator->());
    ES_NO_THROW(t.get());
    ES_NO_THROW((void) !t);
    ES_NO_THROW(t.reset());
    ES_ANY_THROW(t.reset(new TestClass(0xCC)));

    wsd::ScopedPtr<TestClass> t2(new TestClass(0xDD));
    ES_ANY_THROW(t.swap(t2));
}
