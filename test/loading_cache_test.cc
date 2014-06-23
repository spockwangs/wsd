// Copyright (c) 2013 Tencent, Inc.
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#include "loading_cache.h"
#include "gtest/gtest.h"
#include <pthread.h>
#include <iostream>

using namespace wsd;
using namespace std;

Future<int> getInt(const int& i)
{
    return makeFuture<int>(i);
}

Future<int> getException(const int&)
{
    throw "error";
}

TEST(LoadingCache, constructor)
{
    LoadingCache<int, int> cache;
    cache.setLoader(bind(&getInt));
    EXPECT_EQ(0U, cache.size());

    Future<int> fuInt = cache.get(1);
    EXPECT_EQ(1U, cache.size());
    EXPECT_EQ(1, fuInt.get());
    EXPECT_EQ(2, cache.get(2).get());
    EXPECT_EQ(2U, cache.size());

    // An exception was thrown, and the cache size stays the same.
    cache.setLoader(bind(&getException));
    EXPECT_THROW(cache.get(3).get(), const char*);
    EXPECT_EQ(2U, cache.size());
}

Future<int> regetInt(const int&, const int& old)
{
    return makeFuture<int>(old+1);
}

TEST(LoadingCache, refresh)
{
    LoadingCache<int, int> cache;
    cache.refreshAfter(1000);
    cache.setLoader(bind(&getInt), bind(&regetInt));
    cache.get(1);
    sleep(2);
    cache.get(1);
    //EXPECT_EQ(2, cache.get(1).get());
}

Future<int> regetIntException(const int&, const int&)
{
    Promise<int> p;
    p.setException(boost::copy_exception(runtime_error("error")));
    return p.getFuture();
}

TEST(LoadingCache, refresh_exception)
{
    LoadingCache<int, int> cache;
    cache.refreshAfter(1000);
    cache.setLoader(bind(&getInt), bind(&regetIntException));
    cache.get(1);
    sleep(1);

    // Refresh with exception; the value is not updated.
    EXPECT_EQ(1, cache.get(1).get());

    // Now update.
    sleep(1);
    cache.setLoader(bind(&getInt), bind(&regetInt));
    EXPECT_EQ(2, cache.get(1).get());
}

TEST(LoadingCache, capacity)
{
    LoadingCache<int, int> cache;

    // 2 values are inserted.
    cache.setCapacity(2);
    cache.setLoader(bind(&getInt));
    cache.get(1);
    cache.get(2);
    EXPECT_EQ(2U, cache.size());
    EXPECT_EQ(1, cache.get(1).get());  // LRU: 1 2
    EXPECT_EQ(2, cache.get(2).get());  // LRU: 2 1

    // Another value is inserted, which leads to over capacity. So the least-recently-used
    // one (i.e. 1) should be removed.
    cache.get(3);  // LRU: 3 2
    EXPECT_EQ(2U, cache.size());
    EXPECT_EQ(3, cache.get(3).get());  // LRU: 3 2
    EXPECT_EQ(2, cache.get(2).get());  // LRU: 2 3
    EXPECT_FALSE(cache.getIfPresent(1));  // 1 has been removed

    // Now the LRU lis is: 2 3
    // Now insert another one, and 3 should be removed.
    cache.get(4);  // LRU: 4 2
    EXPECT_EQ(2U, cache.size());
    EXPECT_EQ(2, cache.get(2).get());  // LRU: 2 4
    EXPECT_EQ(4, cache.get(4).get());  // LRU: 4 2
    EXPECT_FALSE(cache.getIfPresent(3));
    EXPECT_FALSE(cache.getIfPresent(1));

    // Now LRU: 4 2
    // Now insert another one, and 2 should be removed.
    cache.get(5);  // LRU: 5 4
    EXPECT_EQ(2U, cache.size());
    EXPECT_EQ(5, cache.get(5).get());  // LRU: 5 4
    EXPECT_EQ(4, cache.get(4).get());  // LRU: 4 5
    EXPECT_FALSE(cache.getIfPresent(3));
    EXPECT_FALSE(cache.getIfPresent(2));
    EXPECT_FALSE(cache.getIfPresent(1));
}
    
TEST(LoadingCache, refresh_lru)
{
    LoadingCache<int, int> cache;
    cache.setLoader(bind(&getInt));
    cache.setCapacity(4);
    cache.refreshAfter(500);
    cache.put(1, 1);
    cache.put(2, 2);
    cache.put(3, 3);
    cache.put(4, 4);  // LRU: 4 3 2 1

    sleep(1);
    cache.get(1);  // LRU: 1 4 3 2
    cache.get(5);  // LRU: 5 1 4 3
    EXPECT_FALSE(cache.getIfPresent(2));
}

TEST(LoadingCache, expire)
{
    LoadingCache<int, int> cache;
    cache.expireAfter(1000);
    cache.put(1, 1);
    cache.put(2, 2);
    EXPECT_EQ(2U, cache.size());
    
    // Wait until expiration.
    sleep(1);
    EXPECT_FALSE(cache.getIfPresent(1));
    EXPECT_FALSE(cache.getIfPresent(2));
}

TEST(LoadingCache, put_capacity)
{
    LoadingCache<int, int> cache;
    cache.setCapacity(4);
    cache.put(1, 1);
    cache.put(2, 2);
    cache.put(3, 3);
    cache.put(4, 4);
    cache.put(5, 5);  // 1 was removed
    EXPECT_EQ(4U, cache.size());
    
    EXPECT_FALSE(cache.getIfPresent(1));
    EXPECT_EQ(2, cache.get(2).get());
}

void* setInt(void *p)
{
    sleep(1);
    Promise<int> *promise = reinterpret_cast<Promise<int>*>(p);
    promise->setValue(0xAC);
    delete promise;
    return NULL;
}

Future<int> delayGetInt(const int&)
{
    Promise<int> *p = new Promise<int>();
    pthread_t tid;
    int err = pthread_create(&tid, NULL, &setInt, p);
    EXPECT_EQ(0, err);
    return p->getFuture();
}

TEST(LoadingCache, delay_get_int)
{
    LoadingCache<int, int> cache;
    cache.setLoader(bind(&delayGetInt));
    cache.refreshAfter(1000);
    
    cache.get(1);
    EXPECT_EQ(0xAC, cache.get(1).get());

    sleep(2);  // To force refresh of key 1.
    EXPECT_EQ(0xAC, cache.get(1).get());    
}

TEST(LoadingCache, delay_refresh_exception)
{
    LoadingCache<int, int> cache;
    cache.setLoader(bind(&delayGetInt), bind(&regetIntException));
    cache.refreshAfter(1000);
    
    cache.get(1);
    EXPECT_EQ(0xAC, cache.get(1).get());

    sleep(2);  // To force refresh of key 1.
    EXPECT_EQ(0xAC, cache.get(1).get());

    sleep(2);  // To force refresh of key 1 again.
    EXPECT_EQ(0xAC, cache.get(1).get());
}

TEST(LoadingCache, evictionCount)
{
    LoadingCache<int, int> cache;
    cache.setLoader(bind(&getInt));
    cache.setCapacity(2);
    cache.get(1);
    cache.get(2);
    EXPECT_EQ(0, cache.stats().evictionCount());
    cache.get(3);
    EXPECT_EQ(1, cache.stats().evictionCount());
    cache.get(4);
    EXPECT_EQ(2, cache.stats().evictionCount());
}
