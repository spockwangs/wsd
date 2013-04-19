// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "tuple.h"
#include "gtest/gtest.h"
#include <string>
#include <iostream>

using namespace std;

TEST(tuple, 0)
{
    wsd::Tuple<> t;
    EXPECT_EQ(wsd::Tuple<>(), t);
}

TEST(tuple, 1)
{
    // normal type
    wsd::Tuple<int> t;
    EXPECT_EQ(0, t.get<0>());
    t.get<0>() = 1;
    EXPECT_EQ(1, t.get<0>());

    wsd::Tuple<int> t2(3);
    EXPECT_EQ(3, t2.get<0>());
    t2 = t;
    EXPECT_EQ(t.get<0>(), t2.get<0>());

    // const reference type
    wsd::Tuple<const int&> t3(4);
    EXPECT_EQ(4, t3.get<0>());
    t2 = t3;
    EXPECT_EQ(t3.get<0>(), t2.get<0>());

    // non-const reference type
    int a = 10;
    wsd::Tuple<int&> t4(a);
    EXPECT_EQ(a, t4.get<0>());
    t4.get<0>() = 3;
    EXPECT_EQ(3, a);
    a = 5;
    EXPECT_EQ(5, t4.get<0>());
}

TEST(tuple, 2)
{
    // normal type
    wsd::Tuple<int, int> t;
    EXPECT_EQ(0, t.get<0>());
    EXPECT_EQ(0, t.get<1>());
    t.get<0>() = 3;
    t.get<1>() = 4;
    EXPECT_EQ(3, t.get<0>());
    EXPECT_EQ(4, t.get<1>());

    t = wsd::Tuple<int, int>(6, 5);
    EXPECT_EQ((wsd::Tuple<int, int>(6, 5)), t);

    // reference type
    int a = 3;
    wsd::Tuple<const int&, int&> t2(1, a);
    EXPECT_EQ(1, t2.get<0>());
    EXPECT_EQ(a, t2.get<1>());
    a = 4;
    EXPECT_EQ(1, t2.get<0>());
    EXPECT_EQ(a, t2.get<1>());
    t2.get<1>() = 5;
    EXPECT_EQ(5, a);

    const wsd::Tuple<const int&, int&> t3(1, a);
    EXPECT_EQ(1, t3.get<0>());
    EXPECT_EQ(a, t3.get<1>());
    a = 4;
    EXPECT_EQ(1, t3.get<0>());
    EXPECT_EQ(a, t3.get<1>());
    t3.get<1>() = 5;
    EXPECT_EQ(5, a);
}

TEST(tuple, 3)
{
    wsd::Tuple<int, bool, string> t;
    EXPECT_TRUE(t.get<2>().empty());
    t.get<0>() = 3;
    t.get<1>() = false;
    t.get<2>() = "abc";
    EXPECT_EQ(3, t.get<0>());
    EXPECT_FALSE(t.get<1>());
    EXPECT_EQ("abc", t.get<2>());

    int a = 4;
    string str("xyz");
    wsd::Tuple<const string&, bool, int&> t2(str, true, a);
    EXPECT_EQ("xyz", t2.get<0>());
    EXPECT_TRUE(t2.get<1>());
    EXPECT_EQ(a, t2.get<2>());
    a = 9;
    EXPECT_EQ(9, t2.get<2>());
    t2.get<1>() = 0;
    EXPECT_FALSE(t2.get<1>());
}

TEST(tuple, 10)
{
    string str("hello world");
    wsd::Tuple<bool, char, int, long, float, double, string,
               const int&, wsd::Tuple<string, int>, string&>
        t(false, 'a', 0xdead, 1234567890, 0.5, .001, "xyz",
          4, wsd::Tuple<string, int>(), str);
    EXPECT_FALSE(t.get<0>());
    EXPECT_EQ('a', t.get<1>());
    EXPECT_EQ(0xdead, t.get<2>());
    EXPECT_EQ(1234567890, t.get<3>());
    EXPECT_EQ(0.5, t.get<4>());
    EXPECT_EQ(.001, t.get<5>());
    EXPECT_EQ("xyz", t.get<6>());
    EXPECT_EQ(4, t.get<7>());
    EXPECT_EQ("", t.get<8>().get<0>());
    EXPECT_EQ(0, t.get<8>().get<1>());
    EXPECT_EQ(str, t.get<9>());

    t.get<8>().get<0>() = str;
    EXPECT_EQ(str, t.get<8>().get<0>());
}

TEST(tuple, make_tuple0)
{
    wsd::Tuple<> t = wsd::make_tuple();
    EXPECT_EQ(t, wsd::make_tuple());
}

TEST(tuple, make_tuple1)
{
    wsd::Tuple<int> t = wsd::make_tuple(3);
    EXPECT_EQ(3, t.get<0>());

    const int& i = 4;
    wsd::Tuple<int> t2 = wsd::make_tuple(i);
    EXPECT_EQ(i, t2.get<0>());

    int a = 5;
    int& ra = a;
    wsd::Tuple<int> t3 = wsd::make_tuple(ra);
    EXPECT_EQ(a, t3.get<0>());

    const string& s = "abc";
    wsd::Tuple<string> t4 = wsd::make_tuple(s);
    EXPECT_EQ(s, t4.get<0>());
    t4.get<0>() = "dead";
    EXPECT_EQ("dead", t4.get<0>());
    
    // The following code should not pass the compilation.
    // int c[] = { 1, 2, 3 };
    // wsd::Tuple<int[3]> t5 = wsd::make_tuple(c);
}

TEST(tuple, make_tuple3)
{
    const int& a = 4;
    string s = "abc";
    string& rs = s;
    wsd::Tuple<bool, int, string> t = wsd::make_tuple(false, a, rs);
    EXPECT_FALSE(t.get<0>());
    EXPECT_EQ(a, t.get<1>());
    EXPECT_EQ(s, t.get<2>());
}

TEST(tuple, make_tuple10)
{
    int a = 4;
    string s = "hello, world";
    wsd::Tuple<bool, char, int, long, float, double, string,
               vector<int>, int*, string*> t =
        wsd::make_tuple(true, 'c', 34, 123456789, .1, .02, "xxx", vector<int>(3, 4), &a, &s);
    EXPECT_TRUE(t.get<0>());
    EXPECT_EQ('c', t.get<1>());
    EXPECT_EQ(34, t.get<2>());
    EXPECT_EQ(123456789, t.get<3>());
    EXPECT_EQ(a, *t.get<8>());
    EXPECT_EQ(s, *t.get<9>());

    *t.get<8>() = 18;
    EXPECT_EQ(18, a);
}
