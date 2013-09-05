// Copyright (c) 2013 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "local_cache.h"
#include <string>
#include "gtest/gtest.h"

using namespace std;

class LocalCacheImp : public wsd::LocalCache<int, string> {
public:
    LocalCacheImp(size_t capacity)
        : LocalCache(capacity)
    {}

    LocalCacheImp(size_t capacity, int milli)
        : LocalCache(capacity, milli)
    {}
    
    virtual string load(int i)
    {
        stringstream ss;
        ss << i;
        return ss.str();
    }

    virtual wsd::Future<string> reload(int /*i*/, string s)
    {
        return wsd::makeFuture<string>(s+s);
    }
};

TEST(local_cache, construct)
{
    LocalCacheImp cache(10);
    wsd::SharedPtr<const string> v = cache.get(1);
    EXPECT_EQ("1", *v);
    EXPECT_EQ("1", *cache.get(1));

    v = cache.get(2);
    EXPECT_EQ("2", *v);

    v = cache.get(3);
    EXPECT_EQ("3", *v);

    v = cache.get(1);
    EXPECT_EQ("1", *v);
}

TEST(local_cache, put)
{
    LocalCacheImp cache(10);
    cache.put(1, "123");
    EXPECT_EQ("123", *cache.get(1));

    cache.put(3, "xyz");
    EXPECT_EQ("xyz", *cache.get(3));

    wsd::SharedPtr<const string> v = cache.get(1);
    EXPECT_EQ("123", *v);
    cache.put(1, "xxx");
    EXPECT_EQ("xxx", *cache.get(1));
}

TEST(local_cache, evict)
{
    LocalCacheImp cache(3);
    cache.put(1, "1xx");
    cache.put(2, "2xx");
    cache.put(3, "3xx");
    EXPECT_EQ(3U, cache.size());  // cache: 1, 2, 3

    // now 1 is evicted, but 2 is still there
    cache.put(4, "4");  // cache: 2, 3, 4
    EXPECT_EQ(3U, cache.size());
    EXPECT_EQ("2xx", *cache.get(2));  // cache: 3, 4, 2
    EXPECT_EQ("1", *cache.get(1));    // cache: 4, 2, 1

    cache.put(5, "5xx");           // cache: 2, 1, 5
    EXPECT_EQ(3U, cache.size());
    EXPECT_EQ("2xx", *cache.get(2));  // cache: 1, 5, 2
    EXPECT_EQ("6", *cache.get(6));    // cache: 5, 2, 6
    EXPECT_EQ(3U, cache.size());
    EXPECT_EQ("3", *cache.get(3));  // cache: 2, 6, 3
}

TEST(local_cache, refresh)
{
    LocalCacheImp cache(3, 900);
    cache.put(1, "1");
    EXPECT_EQ(1U, cache.size());
    EXPECT_EQ("1", *cache.get(1));
    EXPECT_EQ("1", *cache.get(1));
    EXPECT_EQ("1", *cache.get(1));
    
    // Wait to be refreshed.
    sleep(1);
    EXPECT_EQ("11", *cache.get(1));

    // Wait to be refreshed again.
    sleep(1);
    EXPECT_EQ("1111", *cache.get(1));
}    

TEST(local_cache, evict_refresh)
{
    LocalCacheImp cache(3, 900);
    cache.put(1, "1xx");
    EXPECT_EQ(1U, cache.size());
    EXPECT_EQ("1xx", *cache.get(1));
    EXPECT_EQ("1xx", *cache.get(1));
    EXPECT_EQ("1xx", *cache.get(1));

    cache.put(2, "2xx");
    cache.put(3, "3xx");

    // Now the key 1 is evicited.
    cache.put(4, "4xx");
    EXPECT_EQ(3U, cache.size());
    EXPECT_EQ("1", *cache.get(1));
    
    // Wait to be refreshed.
    sleep(1);
    EXPECT_EQ("11", *cache.get(1));
    EXPECT_EQ("3xx3xx", *cache.get(3));

    // Wait to be refreshed again.
    sleep(1);
    EXPECT_EQ("1111", *cache.get(1));
}    
