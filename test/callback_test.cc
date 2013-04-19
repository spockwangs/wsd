#include "callback.h"
#include "bind.h"
#include "gtest/gtest.h"
#include <string>
#include <sstream>
#include "es_test.h"

int foo_int(int a)
{
    return a;
}

TEST(callback, bare_func_int)
{
    const wsd::Callback<int ()>& cb = wsd::bind(&foo_int, 10);
    EXPECT_TRUE(cb);
    EXPECT_EQ(10, cb());
    EXPECT_EQ(10, wsd::bind(&foo_int, 10)());
}

int foo_int_ref(int& a)
{
    return a;
}

TEST(callback, bare_func_int_ref)
{
    // We should not pass the compilation to disable bind a non-const reference parameter.
    //wsd::Callback<int ()> cb = wsd::bind(&foo_int_ref, 10);
    //EXPECT_EQ(10, cb());
}

int foo_int_const_ref(const int& a)
{
    return a;
}

TEST(callback, bare_func_int_const_ref)
{
    wsd::Callback<int ()> cb = wsd::bind(&foo_int_const_ref, 10);
    EXPECT_EQ(10, cb());
}

int foo_int_pointer(int *pa)
{
    return *pa;
}

TEST(callback, bare_func_int_pointer)
{
    int a = 10;
    wsd::Callback<int ()> cb = wsd::bind(&foo_int_pointer, wsd::unretained(&a));
    EXPECT_EQ(10, cb());
}

int foo_int_const_pointer(const int *pa)
{
    return *pa;
}

TEST(callback, bare_func_int_const_pointer)
{
    int a = 10;
    wsd::Callback<int()> cb = wsd::bind(&foo_int_const_pointer, wsd::unretained(&a));
    EXPECT_EQ(10, cb());
}

int foo_int_array(int a[2])
{
    return a[0] + a[1];
}

TEST(callback, bare_func_int_array)
{
    int a[2] = { 3, 4 };
    wsd::Callback<int()> cb = wsd::bind(&foo_int_array, wsd::unretained(&a[0]));
    EXPECT_EQ(7, cb());
}

int foo_int_const_array(const int a[2])
{
    return a[0] + a[1];
}

TEST(callback, bare_func_int_const_array)
{
    const int a[2] = { 3, 4 };
    wsd::Callback<int()> cb = wsd::bind(&foo_int_const_array, a);
    EXPECT_EQ(7, cb());
}

class X {
public:
    X(int x) : m_x(x) { }

    int getX() { return m_x; }

private:
    int m_x;
};

TEST(callback, unretained)
{
    X x(3);
    wsd::Callback<int()> cb = wsd::bind(&X::getX, wsd::unretained(&x));
    EXPECT_EQ(3, cb());
}

TEST(callback, owned)
{
    X *p = new X(3);
    wsd::Callback<int()> cb = wsd::bind(&X::getX, wsd::owned(p));
    EXPECT_EQ(3, cb());
}

TEST(callback, shared)
{
    wsd::SharedPtr<X> p(new X(5));
    wsd::Callback<int()> cb = wsd::bind(&X::getX, wsd::shared(p));
    EXPECT_EQ(5, cb());
    EXPECT_EQ(5, p->getX());
}

int foo0()
{
    return 3;
}

int foo1(int a1)
{
    return a1;
}

int foo2(int a1, long a2)
{
    return a1 + a2;
}

int foo3(int a1, int a2, int a3)
{
    return a1 + a2 + a3;
}

int foo4(int a1, int a2, int a3, int a4)
{
    return a1 + a2 + a3 + a4;
}

int foo5(int a1, int a2, int a3, int a4, int a5)
{
    return a1 + a2 + a3 + a4 + a5;
}

int foo6(int a1, int a2, int a3, int a4, int a5, int a6)
{
    return a1 + a2 + a3 + a4 + a5 + a6;
}

int foo7(int a1, int a2, int a3, int a4, int a5, int a6, int a7)
{
    return a1 + a2 + a3 + a4 + a5 + a6 + a7;
}

int foo8(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8)
{
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8;
}

int foo9(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9)
{
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
}

TEST(callback, bare_function0)
{
    wsd::Callback<int ()> cb = wsd::bind(&foo0);
    EXPECT_EQ(3, cb());
}

TEST(callback, bare_function1)
{
    wsd::Callback<int (int)> cb = wsd::bind(&foo1);
    EXPECT_EQ(5, cb(5));
    EXPECT_EQ(5, wsd::bind(&foo1)(5));

    wsd::Callback<int ()> cb2 = wsd::bind(&foo1, 6);
    EXPECT_EQ(6, cb2());
}

TEST(callback, bare_function2)
{
    wsd::Callback<int (int, long)> cb = wsd::bind(&foo2);
    EXPECT_EQ(5, cb(5, 0));

    wsd::Callback<int (long)> cb2 = wsd::bind(&foo2, 6);
    EXPECT_EQ(6, cb2(0));

    wsd::Callback<int ()> cb3 = wsd::bind(&foo2, 3, 4);
    EXPECT_EQ(7, cb3());
}

TEST(callback, bare_function3)
{
    wsd::Callback<int (int, int, int)> cb = wsd::bind(&foo3);
    EXPECT_EQ(6, cb(1, 2, 3));

    wsd::Callback<int (int, int)> cb2 = wsd::bind(&foo3, 1);
    EXPECT_EQ(6, cb2(2, 3));

    wsd::Callback<int (int)> cb3 = wsd::bind(&foo3, 1, 2);
    EXPECT_EQ(6, cb3(3));

    wsd::Callback<int ()> cb4 = wsd::bind(&foo3, 1, 2, 3);
    EXPECT_EQ(6, cb4());
}

TEST(callback, bare_function4)
{
    wsd::Callback<int (int, int, int, int)> cb = wsd::bind(&foo4);
    EXPECT_EQ(10, cb(1, 2, 3, 4));

    wsd::Callback<int (int, int, int)> cb2 = wsd::bind(&foo4, 1);
    EXPECT_EQ(10, cb2(2, 3, 4));

    wsd::Callback<int (int, int)> cb3 = wsd::bind(&foo4, 1, 2);
    EXPECT_EQ(10, cb3(3, 4));

    wsd::Callback<int (int)> cb4 = wsd::bind(&foo4, 1, 2, 3);
    EXPECT_EQ(10, cb4(4));

    wsd::Callback<int ()> cb5 = wsd::bind(&foo4, 1, 2, 3, 4);
    EXPECT_EQ(10, cb5());
}

TEST(callback, bare_function5)
{
    wsd::Callback<int (int, int, int, int, int)> cb = wsd::bind(&foo5);
    EXPECT_EQ(15, cb(1, 2, 3, 4, 5));

    wsd::Callback<int (int, int, int, int)> cb1 = wsd::bind(&foo5, 1);
    EXPECT_EQ(15, cb1(2, 3, 4, 5));

    wsd::Callback<int (int, int, int)> cb2 = wsd::bind(&foo5, 1, 2);
    EXPECT_EQ(15, cb2(3, 4, 5));

    wsd::Callback<int (int, int)> cb3 = wsd::bind(&foo5, 1, 2, 3);
    EXPECT_EQ(15, cb3(4, 5));

    wsd::Callback<int (int)> cb4 = wsd::bind(&foo5, 1, 2, 3, 4);
    EXPECT_EQ(15, cb4(5));

    wsd::Callback<int ()> cb5 = wsd::bind(&foo5, 1, 2, 3, 4, 5);
    EXPECT_EQ(15, cb5());
}

TEST(callback, bare_function6)
{
    wsd::Callback<int (int, int, int, int, int, int)> cb = wsd::bind(&foo6);
    EXPECT_EQ(21, cb(1, 2, 3, 4, 5, 6));

    wsd::Callback<int (int, int, int, int, int)> cb1 = wsd::bind(&foo6, 1);
    EXPECT_EQ(21, cb1(2, 3, 4, 5, 6));

    wsd::Callback<int (int, int, int, int)> cb2 = wsd::bind(&foo6, 1, 2);
    EXPECT_EQ(21, cb2(3, 4, 5, 6));

    wsd::Callback<int (int, int, int)> cb3 = wsd::bind(&foo6, 1, 2, 3);
    EXPECT_EQ(21, cb3(4, 5, 6));

    wsd::Callback<int (int, int)> cb4 = wsd::bind(&foo6, 1, 2, 3, 4);
    EXPECT_EQ(21, cb4(5, 6));

    wsd::Callback<int (int)> cb5 = wsd::bind(&foo6, 1, 2, 3, 4, 5);
    EXPECT_EQ(21, cb5(6));

    wsd::Callback<int ()> cb6 = wsd::bind(&foo6, 1, 2, 3, 4, 5, 6);
    EXPECT_EQ(21, cb6());
}

TEST(callback, bare_function7)
{
    wsd::Callback<int (int, int, int, int, int, int, int)> cb = wsd::bind(&foo7);
    EXPECT_EQ(28, cb(1, 2, 3, 4, 5, 6, 7));

    wsd::Callback<int (int, int, int, int, int, int)> cb1 = wsd::bind(&foo7, 1);
    EXPECT_EQ(28, cb1(2, 3, 4, 5, 6, 7));

    wsd::Callback<int (int, int, int, int, int)> cb2 = wsd::bind(&foo7, 1, 2);
    EXPECT_EQ(28, cb2(3, 4, 5, 6, 7));

    wsd::Callback<int (int, int, int, int)> cb3 = wsd::bind(&foo7, 1, 2, 3);
    EXPECT_EQ(28, cb3(4, 5, 6, 7));

    wsd::Callback<int (int, int, int)> cb4 = wsd::bind(&foo7, 1, 2, 3, 4);
    EXPECT_EQ(28, cb4(5, 6, 7));

    wsd::Callback<int (int, int)> cb5 = wsd::bind(&foo7, 1, 2, 3, 4, 5);
    EXPECT_EQ(28, cb5(6, 7));

    wsd::Callback<int (int)> cb6 = wsd::bind(&foo7, 1, 2, 3, 4, 5, 6);
    EXPECT_EQ(28, cb6(7));

    wsd::Callback<int ()> cb7 = wsd::bind(&foo7, 1, 2, 3, 4, 5, 6, 7);
    EXPECT_EQ(28, cb7());
}

TEST(callback, bare_function8)
{
    wsd::Callback<int (int, int, int, int, int, int, int, int)> cb = wsd::bind(&foo8);
    EXPECT_EQ(36, cb(1, 2, 3, 4, 5, 6, 7, 8));

    wsd::Callback<int (int, int, int, int, int, int, int)> cb1 = wsd::bind(&foo8, 1);
    EXPECT_EQ(36, cb1(2, 3, 4, 5, 6, 7, 8));

    wsd::Callback<int (int, int, int, int, int, int)> cb2 = wsd::bind(&foo8, 1, 2);
    EXPECT_EQ(36, cb2(3, 4, 5, 6, 7, 8));

    wsd::Callback<int (int, int, int, int, int)> cb3 = wsd::bind(&foo8, 1, 2, 3);
    EXPECT_EQ(36, cb3(4, 5, 6, 7, 8));

    wsd::Callback<int (int, int, int, int)> cb4 = wsd::bind(&foo8, 1, 2, 3, 4);
    EXPECT_EQ(36, cb4(5, 6, 7, 8));

    wsd::Callback<int (int, int, int)> cb5 = wsd::bind(&foo8, 1, 2, 3, 4, 5);
    EXPECT_EQ(36, cb5(6, 7, 8));

    wsd::Callback<int (int, int)> cb6 = wsd::bind(&foo8, 1, 2, 3, 4, 5, 6);
    EXPECT_EQ(36, cb6(7, 8));

    wsd::Callback<int (int)> cb7 = wsd::bind(&foo8, 1, 2, 3, 4, 5, 6, 7);
    EXPECT_EQ(36, cb7(8));

    wsd::Callback<int ()> cb8 = wsd::bind(&foo8, 1, 2, 3, 4, 5, 6, 7, 8);
    EXPECT_EQ(36, cb8());
}

TEST(callback, bare_function9)
{
    wsd::Callback<int (int, int, int, int, int, int, int, int, int)> cb = wsd::bind(&foo9);
    EXPECT_EQ(45, cb(1, 2, 3, 4, 5, 6, 7, 8, 9));

    wsd::Callback<int (int, int, int, int, int, int, int, int)> cb1 = wsd::bind(&foo9, 1);
    EXPECT_EQ(45, cb1(2, 3, 4, 5, 6, 7, 8, 9));

    wsd::Callback<int (int, int, int, int, int, int, int)> cb2 = wsd::bind(&foo9, 1, 2);
    EXPECT_EQ(45, cb2(3, 4, 5, 6, 7, 8, 9));

    wsd::Callback<int (int, int, int, int, int, int)> cb3 = wsd::bind(&foo9, 1, 2, 3);
    EXPECT_EQ(45, cb3(4, 5, 6, 7, 8, 9));

    wsd::Callback<int (int, int, int, int, int)> cb4 = wsd::bind(&foo9, 1, 2, 3, 4);
    EXPECT_EQ(45, cb4(5, 6, 7, 8, 9));

    wsd::Callback<int (int, int, int, int)> cb5 = wsd::bind(&foo9, 1, 2, 3, 4, 5);
    EXPECT_EQ(45, cb5(6, 7, 8, 9));

    wsd::Callback<int (int, int, int)> cb6 = wsd::bind(&foo9, 1, 2, 3, 4, 5, 6);
    EXPECT_EQ(45, cb6(7, 8, 9));

    wsd::Callback<int (int, int)> cb7 = wsd::bind(&foo9, 1, 2, 3, 4, 5, 6, 7);
    EXPECT_EQ(45, cb7(8, 9));

    wsd::Callback<int (int)> cb8 = wsd::bind(&foo9, 1, 2, 3, 4, 5, 6, 7, 8);
    EXPECT_EQ(45, cb8(9));

    wsd::Callback<int ()> cb9 = wsd::bind(&foo9, 1, 2, 3, 4, 5, 6, 7, 8, 9);
    EXPECT_EQ(45, cb9());
}

class Foo {
public:
    int get0()
    { return 3; }

    int get1(int a1)
    { return a1; }

    int get2(int a1, int a2)
    { return a1 + a2; }
};

TEST(callback, function_object0)
{
    wsd::Callback<int (Foo*)> cb = wsd::bind(&Foo::get0);
    Foo foo;
    EXPECT_EQ(3, cb(&foo));

    wsd::Callback<int ()> cb2 = wsd::bind(&Foo::get0, wsd::owned(new Foo()));
    EXPECT_EQ(3, cb2());
}

TEST(callback, function_object1)
{
    wsd::Callback<int (Foo*, int)> cb = wsd::bind(&Foo::get1);
    Foo foo;
    EXPECT_EQ(4, cb(&foo, 4));

    wsd::Callback<int (int)> cb2 = wsd::bind(&Foo::get1, wsd::owned(new Foo()));
    EXPECT_EQ(7, cb2(7));

    wsd::Callback<int ()> cb3 = wsd::bind(&Foo::get1, wsd::owned(new Foo()), 3);
    EXPECT_EQ(3, cb3());
}

TestClass run(TestClass t)
{
    t.run();
    return t;
}

TestClass run_cref(const TestClass& t)
{
    t.run();
    return t;
}

TestClass run_ref(TestClass& t)
{
    t.run();
    return t;
}

TEST(callback, exception_safety)
{
    ES_ANY_THROW(wsd::bind(&run)(TestClass()));
    ES_ANY_THROW(wsd::bind(&run, TestClass())());

    TestClass t;
    ES_ANY_THROW(wsd::bind(&run_cref)(t));
    ES_ANY_THROW(wsd::bind(&run_cref, t)());
    
    ES_ANY_THROW(wsd::bind(&run_ref)(t));

    ES_ANY_THROW(wsd::bind(&TestClass::run, wsd::shared(wsd::SharedPtr<TestClass>(new TestClass()))));
    ES_ANY_THROW(wsd::bind(&TestClass::run, wsd::owned(new TestClass())));
    ES_ANY_THROW(wsd::bind(&TestClass::run, wsd::unretained(&t)));
}
