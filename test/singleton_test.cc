// Copyright (c) 2013 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "singleton.h"

#include "gtest/gtest.h"

class TestClass {
public:
    TestClass() : m_p(new int())
    {
    }

    int *m_p;
};

TEST(Singleton, constructor)
{
    // Test that a instance is created.
    boost::shared_ptr<TestClass> p1 = wsd::Singleton<TestClass>::getInstance();
    EXPECT_TRUE(p1 != nullptr);

    // Test that 2 instances are the same.
    boost::shared_ptr<TestClass> p2 = wsd::Singleton<TestClass>::getInstance();
    EXPECT_EQ(p1, p2);
    EXPECT_TRUE(p1->m_p);
    EXPECT_EQ(p1->m_p, p2->m_p);

    // Test that the shared pointer does not delete the instance.
    TestClass *p = p1.get();
    p1.reset();
    p2.reset();
    p1 = wsd::Singleton<TestClass>::getInstance();
    p2 = wsd::Singleton<TestClass>::getInstance();
    EXPECT_EQ(p, p1.get());
    EXPECT_EQ(p1, p2);
    EXPECT_EQ(p1->m_p, p2->m_p);
}
