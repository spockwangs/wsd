// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "concurrent_map.h"
#include "gtest/gtest.h"
#include "es_test.h"
#include <algorithm>

TEST(concurrent_map, construct)
{
    wsd::ConcurrentMap<int, int> m;
    EXPECT_EQ(0U, m.size());
    EXPECT_TRUE(m.empty());
    EXPECT_EQ(0U, m.count(1));

    std::map<int, int> c;
    c.insert(std::make_pair(1, 1));
    c.insert(std::make_pair(2, 1));
    c.insert(std::make_pair(3, 1));
    c.insert(std::make_pair(4, 1));
    wsd::ConcurrentMap<int, int> m2(c.begin(), c.end());
    EXPECT_EQ(c.size(), m2.size());
    //EXPECT_TRUE(std::equal(c.begin(), c.end(), m2.begin()));
    EXPECT_FALSE(m2.empty());
    EXPECT_TRUE(m2.count(1));
    EXPECT_TRUE(m2.count(2));
    EXPECT_TRUE(m2.count(3));
    EXPECT_TRUE(m2.count(4));

    wsd::ConcurrentMap<int, int> m3(m2);
    EXPECT_EQ(m2.empty(), m3.empty());
    EXPECT_EQ(m2.size(), m3.size());
    EXPECT_TRUE(m3.count(1));
    EXPECT_TRUE(m3.count(2));
    EXPECT_TRUE(m3.count(3));
    EXPECT_TRUE(m3.count(4));

    m3.clear();
    EXPECT_TRUE(m3.empty());
    m3 = m2;
    EXPECT_EQ(m2.empty(), m3.empty());
    EXPECT_EQ(m2.size(), m3.size());
    EXPECT_TRUE(m3.count(1));
    EXPECT_TRUE(m3.count(2));
    EXPECT_TRUE(m3.count(3));
    EXPECT_TRUE(m3.count(4));
}

TEST(concurrent_map, erase)
{
    wsd::ConcurrentMap<int, int> m;
    EXPECT_FALSE(m.erase(3));

    EXPECT_TRUE(m.insert(std::make_pair(3, 3)));
    EXPECT_EQ(1U, m.size());
    EXPECT_FALSE(m.empty());
    EXPECT_TRUE(m.erase(3));
    EXPECT_EQ(0U, m.size());
    EXPECT_TRUE(m.empty());

    wsd::ConcurrentMap<int, int>::ConstAccessor const_accessor;
    EXPECT_TRUE(m.insert(std::make_pair(3, 3), &const_accessor));
    EXPECT_EQ(1U, m.size());
    EXPECT_FALSE(m.empty());
    EXPECT_TRUE(m.erase(&const_accessor));
    EXPECT_EQ(0U, m.size());
    EXPECT_TRUE(m.empty());
    EXPECT_EQ(0U, m.count(3));

    wsd::ConcurrentMap<int, int>::Accessor accessor;
    EXPECT_TRUE(m.insert(std::make_pair(3, 3), &accessor));
    EXPECT_EQ(1U, m.size());
    EXPECT_FALSE(m.empty());
    EXPECT_TRUE(m.erase(&accessor));
    EXPECT_EQ(0U, m.size());
    EXPECT_TRUE(m.empty());
    EXPECT_EQ(0U, m.count(3));
}

TEST(concurrent_map, swap)
{
    wsd::ConcurrentMap<int, int> m;
    m.insert(std::make_pair(1, 1));
    m.insert(std::make_pair(2, 1));
    m.insert(std::make_pair(3, 1));
    m.insert(std::make_pair(4, 1));
    m.insert(std::make_pair(5, 1));

    wsd::ConcurrentMap<int, int> m2;
    m2.insert(std::make_pair(9, 2));

    m.swap(m2);
    EXPECT_EQ(5U, m2.size());
    EXPECT_EQ(1U, m.size());
    EXPECT_TRUE(m2.count(1));
    EXPECT_TRUE(m2.count(2));
    EXPECT_TRUE(m2.count(3));
    EXPECT_TRUE(m2.count(4));
    EXPECT_TRUE(m2.count(5));
    EXPECT_TRUE(m.count(9));
}

TEST(concurrent_map, const_accessor)
{
    wsd::ConcurrentMap<int, int> m;
    EXPECT_EQ(0U, m.count(1));

    wsd::ConcurrentMap<int, int>::ConstAccessor const_accessor;
    EXPECT_FALSE(m.find(1, &const_accessor));
    EXPECT_TRUE(const_accessor.empty());
    
    EXPECT_TRUE(m.insert(std::make_pair(1, 1)));
    EXPECT_EQ(1U, m.size());
    EXPECT_EQ(1U, m.count(1));
    
    EXPECT_TRUE(m.find(1, &const_accessor));
    EXPECT_FALSE(const_accessor.empty());
    EXPECT_EQ(1, const_accessor->first);
    EXPECT_EQ(1, const_accessor->second);
    EXPECT_TRUE(m.find(1, &const_accessor));

    EXPECT_TRUE(m.insert(std::make_pair(2, 2), &const_accessor));
    EXPECT_EQ(2, const_accessor->first);
    EXPECT_EQ(2, const_accessor->second);
    EXPECT_EQ(2U, m.size());

    // Now there should be no lock on the pair(1, x).
    EXPECT_TRUE(m.find(1, &const_accessor));
    
    EXPECT_FALSE(m.insert(2, &const_accessor));
    EXPECT_EQ(2, const_accessor->second);

    EXPECT_TRUE(m.insert(3, &const_accessor));
    EXPECT_EQ(0, const_accessor->second);
}

TEST(concurrent_map, accessor)
{
    wsd::ConcurrentMap<int, int> m;
    EXPECT_EQ(0U, m.count(1));

    wsd::ConcurrentMap<int, int>::Accessor accessor;
    EXPECT_FALSE(m.find(1, &accessor));
    EXPECT_TRUE(accessor.empty());
    
    EXPECT_TRUE(m.insert(std::make_pair(1, 1)));
    EXPECT_EQ(1U, m.size());
    EXPECT_EQ(1U, m.count(1));
    
    EXPECT_TRUE(m.find(1, &accessor));
    EXPECT_FALSE(accessor.empty());
    EXPECT_EQ(1, accessor->first);
    EXPECT_EQ(1, accessor->second);
    EXPECT_TRUE(m.find(1, &accessor));

    EXPECT_TRUE(m.insert(std::make_pair(2, 2), &accessor));
    EXPECT_EQ(2, accessor->first);
    EXPECT_EQ(2, accessor->second);
    EXPECT_EQ(2U, m.size());

    // Now there should be no lock on the pair(1, x).
    EXPECT_TRUE(m.find(1, &accessor));
    
    EXPECT_FALSE(m.insert(2, &accessor));
    EXPECT_EQ(2, accessor->second);

    EXPECT_TRUE(m.insert(3, &accessor));
    EXPECT_EQ(0, accessor->second);
}

TEST(concurrent_map, iteartor)
{
    wsd::ConcurrentMap<int, int> m;
    EXPECT_EQ(0, std::distance(m.begin(), m.end()));

    std::vector<std::pair<int, int> > v;
    int n = std::rand() % 10000;
    for (int i = 0; i < n; i++)
        v.push_back(std::make_pair(i, std::rand()));
    
    wsd::ConcurrentMap<int, int> m2(v.begin(), v.end());
    for (wsd::ConcurrentMap<int, int>::const_iterator it = m2.begin(); it != m2.end(); ++it)
        EXPECT_TRUE(std::find(v.begin(), v.end(), std::make_pair(it->first, it->second)) != v.end());
    EXPECT_EQ(v.size(), (size_t) std::distance(m2.begin(), m2.end()));

    const wsd::ConcurrentMap<int, int> m3;
    EXPECT_EQ(0, std::distance(m3.begin(), m3.end()));

    wsd::ConcurrentMap<int, int> m4(v.begin(), v.end());
    for (wsd::ConcurrentMap<int, int>::const_iterator it = m4.begin(); it != m4.end(); ++it)
        EXPECT_TRUE(std::find(v.begin(), v.end(), std::make_pair(it->first, it->second)) != v.end());
    EXPECT_EQ(v.size(), (size_t) std::distance(m4.begin(), m4.end()));

}

template <typename K, typename V>
bool checkInvariant(const wsd::ConcurrentMap<K, V>& c)
{
    if (c.empty() && c.size() > 0)
        return false;

    if (!c.empty() && c.size() == 0)
        return false;

    if ((size_t) std::distance(c.begin(), c.end()) != c.size())
        return false;
    
    return true;
}
            
struct test_construct {
    void operator()(int) const
    {
        wsd::ConcurrentMap<TestClass, TestClass> m;
        EXPECT_TRUE(m.empty());
        EXPECT_EQ(0U, m.size());
    }
};
    
struct test_construct_iterator {
    void operator()(int) const
    {
        std::vector<std::pair<TestClass, TestClass> > v;
        for (size_t i = 0, size = std::rand() % 100; i < size; i++)
            v.push_back(std::make_pair(TestClass(i), TestClass()));

        wsd::ConcurrentMap<TestClass, TestClass> m(v.begin(), v.end());
        EXPECT_EQ(v.size(), m.size());
        for (size_t i = 0; i < v.size(); i++)
            EXPECT_EQ(1U, m.count(i));
    }
};

struct test_empty {
    test_empty(const wsd::ConcurrentMap<TestClass, TestClass>& orig)
        : orig(orig)
    {}

    void operator()(int) const
    {
        orig.empty();
    }

    wsd::ConcurrentMap<TestClass, TestClass> orig;
};

struct test_size {
    test_size(const wsd::ConcurrentMap<TestClass, TestClass>& orig)
        : orig(orig)
    {}

    void operator()(size_t n) const
    {
        EXPECT_EQ(n, orig.size());
    }

    wsd::ConcurrentMap<TestClass, TestClass> orig;
};

struct test_count {
    test_count(const wsd::ConcurrentMap<TestClass, TestClass>& orig)
        : orig(orig)
    {}

    void operator()(int) const
    {
        orig.count(3);
    }

    wsd::ConcurrentMap<TestClass, TestClass> orig;
};

struct test_find {
    test_find(const TestClass& key)
        : key(key)
    {}

    void operator()(wsd::ConcurrentMap<TestClass, TestClass>& t) const
    {
        wsd::ConcurrentMap<TestClass, TestClass>::ConstAccessor const_accessor;
        wsd::ConcurrentMap<TestClass, TestClass>::Accessor accessor;
        EXPECT_TRUE(t.find(key, &const_accessor));
        const_accessor.release();
        EXPECT_TRUE(t.find(key, &accessor));
        accessor.release();
        
        t.erase(key);
        EXPECT_FALSE(t.find(key, &const_accessor));
        EXPECT_FALSE(t.find(key, &accessor));
    }

    TestClass key;
};

struct test_insert {
    test_insert(const wsd::ConcurrentMap<TestClass, TestClass>& orig)
        : orig(orig)
    {
        insertValue.first = TestClass(std::rand());
        insertValue.second = TestClass(std::rand());
    }

    void operator()(wsd::ConcurrentMap<TestClass, TestClass>& t) const
    {
        bool inserted = t.insert(insertValue);

        g_throw_counter = -1;
        if (inserted)
            EXPECT_EQ(orig.size() + 1, t.size());
        else
            EXPECT_EQ(orig.size(), t.size());
        EXPECT_EQ(1U, t.count(insertValue.first));
    }

    wsd::ConcurrentMap<TestClass, TestClass> orig;
    std::pair<TestClass, TestClass> insertValue;
};

struct test_insert_value_const_accessor {
    test_insert_value_const_accessor(const wsd::ConcurrentMap<TestClass, TestClass>& orig)
        : orig(orig)
    {
        insertValue.first = TestClass(std::rand());
        insertValue.second = TestClass(std::rand());
    }

    void operator()(wsd::ConcurrentMap<TestClass, TestClass>& t) const
    {
        wsd::ConcurrentMap<TestClass, TestClass>::ConstAccessor const_accessor;
        bool inserted = t.insert(insertValue, &const_accessor);

        g_throw_counter = -1;
        if (inserted)
            EXPECT_EQ(orig.size() + 1, t.size());
        else
            EXPECT_EQ(orig.size(), t.size());
        EXPECT_EQ(1U, t.count(insertValue.first));
        EXPECT_EQ(insertValue.first, const_accessor->first);
        EXPECT_EQ(insertValue.second, const_accessor->second);
    }

    wsd::ConcurrentMap<TestClass, TestClass> orig;
    std::pair<TestClass, TestClass> insertValue;
};

struct test_insert_key_const_accessor {
    test_insert_key_const_accessor(const wsd::ConcurrentMap<TestClass, TestClass>& orig)
        : orig(orig)
    {
        insertKey = TestClass(std::rand());
    }

    void operator()(wsd::ConcurrentMap<TestClass, TestClass>& t) const
    {
        wsd::ConcurrentMap<TestClass, TestClass>::ConstAccessor const_accessor;
        bool inserted = t.insert(insertKey, &const_accessor);

        g_throw_counter = -1;
        if (inserted)
            EXPECT_EQ(orig.size() + 1, t.size());
        else
            EXPECT_EQ(orig.size(), t.size());
        EXPECT_EQ(1U, t.count(insertKey));
        EXPECT_EQ(insertKey, const_accessor->first);
        EXPECT_EQ(TestClass(), const_accessor->second);
    }

    wsd::ConcurrentMap<TestClass, TestClass> orig;
    TestClass insertKey;
};

struct test_insert_value_accessor {
    test_insert_value_accessor(const wsd::ConcurrentMap<TestClass, TestClass>& orig)
        : orig(orig)
    {
        insertValue.first = TestClass(std::rand());
        insertValue.second = TestClass(std::rand());
    }

    void operator()(wsd::ConcurrentMap<TestClass, TestClass>& t) const
    {
        wsd::ConcurrentMap<TestClass, TestClass>::Accessor accessor;
        bool inserted = t.insert(insertValue, &accessor);

        g_throw_counter = -1;
        if (inserted)
            EXPECT_EQ(orig.size() + 1, t.size());
        else
            EXPECT_EQ(orig.size(), t.size());
        EXPECT_EQ(1U, t.count(insertValue.first));
        EXPECT_EQ(insertValue.first, accessor->first);
        EXPECT_EQ(insertValue.second, accessor->second);
    }

    wsd::ConcurrentMap<TestClass, TestClass> orig;
    std::pair<TestClass, TestClass> insertValue;
};

struct test_insert_key_accessor {
    test_insert_key_accessor(const wsd::ConcurrentMap<TestClass, TestClass>& orig)
        : orig(orig)
    {
        insertKey = TestClass(std::rand());
    }

    void operator()(wsd::ConcurrentMap<TestClass, TestClass>& t) const
    {
        wsd::ConcurrentMap<TestClass, TestClass>::Accessor accessor;
        bool inserted = t.insert(insertKey, &accessor);

        g_throw_counter = -1;
        if (inserted)
            EXPECT_EQ(orig.size() + 1, t.size());
        else
            EXPECT_EQ(orig.size(), t.size());
        EXPECT_EQ(1U, t.count(insertKey));
        EXPECT_EQ(insertKey, accessor->first);
        EXPECT_EQ(TestClass(), accessor->second);
    }

    wsd::ConcurrentMap<TestClass, TestClass> orig;
    TestClass insertKey;
};

struct test_erase {
    test_erase(const TestClass& key)
        : key(key)
    {
    }

    void operator()(wsd::ConcurrentMap<TestClass, TestClass>& t) const
    {
        bool erased = t.erase(key);
        EXPECT_TRUE(erased);
    }

    TestClass key;
};

TEST(concurrent_map, exception_safety)
{
    strongCheck(0, test_construct());
    strongCheck(0, test_construct_iterator());

    wsd::ConcurrentMap<TestClass, TestClass> m;
    nothrowCheck(0, test_empty(m));
    nothrowCheck(0U, test_size(m));
    strongCheck(0, test_count(m));

    m.insert(std::make_pair(TestClass(1), TestClass()));
    nothrowCheck(m, test_find(TestClass(1)));

    wsd::ConcurrentMap<TestClass, TestClass> m2;
    strongCheck(m2, test_insert(m2));
    strongCheck(m2, test_insert_value_const_accessor(m2));
    strongCheck(m2, test_insert_key_const_accessor(m2));
    strongCheck(m2, test_insert_value_accessor(m2));
    strongCheck(m2, test_insert_key_accessor(m2));

    m2.insert(std::make_pair(TestClass(3), TestClass()));
    nothrowCheck(m2, test_erase(TestClass(3)));
}
