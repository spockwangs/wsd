// Copyright (c) 2013 wbb
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef WSD_LOADING_CACHE_H
#define WSD_LOADING_CACHE_H

#include <time.h>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include "promise.h"

namespace wsd {

    template <typename K, typename V> class LoadingCache;

    class CacheStats {
    public:
        /**
         * Returns the average time spent loading new values. This is defined as
         * `totalLoadTime / (loadSuccessCount + loadExceptionCount)`, or 0.0 when
         * `loadSuccessCount + loadExceptionCount == 0`.
         *
         * @note Not implemented yet.
         */
        double averageLoadPenalty() const
        {
            int64_t totalLoadCount = m_loadSuccessCount + m_loadSuccessCount;
            return (totalLoadCount == 0)
                ? 0.0
                :  static_cast<double>(m_totalLoadTime) / totalLoadCount;
        }

        /**
         * Returns the number of times an entry has been evicted. This count does not
         * include manual invalidations.
         */
        int64_t evictionCount() const
        {
            return m_evictionCount;
        }

        /**
         * Returns the number of times cache lookup methods have returned a cached value.
         */
        int64_t hitCount() const
        {
            return m_hitCount;
        }

        /**
         * Returns the ratio of cache requests which were hits. This is defined as
         * `hitCount / requestCount`, or `1.0` when `requestCount == 0`. Note that `hitRate +
         * missRate =~ 1.0`.
         */
        double hitRate() const
        {
            int64_t n = requestCount();
            return n == 0 ? 1.0 : static_cast<double>(m_hitCount) / n;
        }

        /**
         * Returns the total number of times that cache lookup methods attempted to load
         * new values. This includes both successful load operations, as well as those
         * that threw exceptions. This is defined as `loadSuccessCount +
         * loadExceptionCount`.
         *
         * @note Not implemented yet.
         */
        int64_t loadCount() const
        {
            return m_loadSuccessCount + m_loadExceptionCount;
        }

        /**
         * @note Not implemented yet.
         */
        int64_t loadExceptionCount() const
        {
            return m_loadExceptionCount;
        }

        /**
         * @note Not implemented yet.
         */
        double loadExceptionRate() const
        {
            int64_t totalLoadCount = m_loadSuccessCount + m_loadExceptionCount;
            return (totalLoadCount == 0)
                ? 0.0
                : static_cast<double>(m_loadExceptionCount) / totalLoadCount;
        }

        /**
         * @note Not implemented yet.
         */
        int64_t loadSuccessCount() const
        {
            return m_loadSuccessCount;
        }

        /**
         * Returns the number of times cache lookup methods have returned an uncached
         * (newly loaded) value, or null. Multiple concurrent calls to cache lookup
         * methods on an absent value can result in multiple misses, all returning the
         * results of a single cache load operation.
         */
        int64_t missCount() const
        {
            return m_missCount;
        }

        /**
         * Returns the ratio of cache requests which were misses. This is defined as
         * `missCount / requestCount`, or 0.0 when `requestCount == 0`. Note that `hitRate +
         * missRate =~ 1.0`. Cache misses include all requests which weren't cache hits,
         * including requests which resulted in either successful or failed loading
         * attempts, and requests which waited for other threads to finish loading.
         */
        double missRate() const
        {
            int64_t n = requestCount();
            return n == 0 ? 0.0 : static_cast<double>(m_missCount) / n;
        }

        /**
         * Returns the number of times Cache lookup methods have returned either a cached
         * or uncached value. This is defined as `hitCount + missCount`.
         */
        int64_t requestCount() const
        {
            return m_hitCount + m_missCount;
        }

        /**
         * @note Not implemented yet.
         */
        int64_t totalLoadTime() const
        {
            return m_totalLoadTime;
        }

        std::string toString() const
        {
            std::stringstream ss;
            ss << "request count: " << requestCount()
               << "\nhit rate: " << std::setprecision(2) << std::fixed << hitRate() * 100 << "%"
               << "\neviction count: " << evictionCount();
            return ss.str();
        }

    private:
        template <typename K, typename V> friend class LoadingCache;

        CacheStats()
            : m_hitCount(0), m_missCount(0), m_loadSuccessCount(0),
              m_loadExceptionCount(0), m_totalLoadTime(0), m_evictionCount(0)
        {}

        int64_t m_hitCount;
        int64_t m_missCount;
        int64_t m_loadSuccessCount;
        int64_t m_loadExceptionCount;
        int64_t m_totalLoadTime;
        int64_t m_evictionCount;
    };

    template <typename K, typename V>
    class LoadingCache {
    public:
        typedef Callback<Future<V>(const K&)> Loader;
        typedef Callback<Future<V>(const K&, const V&)> Reloader;

        LoadingCache()
            : m_listHead(m_map.end()),
              m_expireMilliseconds(0),
              m_refreshInterval(0),
              m_capacity(0)
        {
        }

        void setLoader(const Loader& loader, const Reloader& reloader = Reloader())
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            m_loader = loader;
            m_reloader = reloader;
        }

        void refreshAfter(int64_t milliseconds)
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            m_refreshInterval = milliseconds;
        }

        void expireAfter(int64_t milliseconds)
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            m_expireMilliseconds = milliseconds;
        }

        void setCapacity(size_t n)
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            m_capacity = n;
        }

        size_t size() const
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            return m_map.size();
        }

        Future<V> get(const K& key);

        /**
         * Returns the associated value if present, or uninitialized future otherwise.
         */
        Future<V> getIfPresent(const K& key);

        void put(const K& key, const V& value);

        void invalidate(const K& key);

        void invalidateAll();

        CacheStats stats() const
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            return m_cacheStats;
        }

    private:
        struct Object;
        typedef std::map<K, Object> M;

        // States of cache objects:
        //   loading: newVal is loading and oldVal does not exist
        //   valid: newVal is valid or oldVal is valid or both are valid (newVal take priority)
        //   refreshing: oldVal is valid and newVal is loading
        //   invalid: oldVal and newVal are both invalid
        struct Object {
            Object()
                : writeTime(0),
                  prev(NULL),
                  next(NULL)
            {}

            Object(const V& value, int64_t writeTime)
                : newVal(makeFuture<V>(value)),
                  writeTime(writeTime),
                  prev(NULL),
                  next(NULL)
            {}

            bool isValid() const
            {
                return newVal.hasValue() || (newVal.hasException() && oldVal.hasValue());
            }

            bool isLoading() const
            {
                return !oldVal && newVal && !newVal.isDone();
            }

            bool isRefreshing() const
            {
                return oldVal.hasValue() && newVal && !newVal.isDone();
            }

            bool isRefreshFailure() const
            {
                return oldVal.hasValue() && newVal.hasException();
            }

            Future<V> getValue() const
            {
                if (isRefreshing())
                    return oldVal;
                BOOST_ASSERT(isValid() || isLoading());
                if (isLoading() || newVal.hasValue())
                    return newVal;
                return oldVal;
            }

            int64_t getWriteTime() const
            {
                return writeTime;
            }

            void refresh(const Future<V>& val, int64_t now)
            {
                if (newVal.hasValue())
                    oldVal = newVal;
                newVal = val;
                writeTime = now;
            }

            Future<V> oldVal;
            Future<V> newVal;
            int64_t writeTime;
            typename M::iterator prev, next;  // Link to LRU list
        };

        typename M::iterator getLiveObj(const K& key, int64_t now);

        void scheduleRefresh(int64_t now, typename M::iterator it);

        Future<V> load(const K& key, int64_t now);

        void remove(typename M::iterator it);

        void evictEntries();

        void markAccess(typename M::iterator it);

        void addToFront(typename M::iterator it);

        void removeFromAccessList(typename M::iterator it);

        int64_t getTick()
        {
            return time(NULL) * 1000;
        }

        mutable boost::mutex m_mutex;
        M m_map;
        typename M::iterator m_listHead;  // Link all cache objects to a LRU list.
        int64_t m_expireMilliseconds;
        int64_t m_refreshInterval;
        size_t m_capacity;
        Loader m_loader;
        Reloader m_reloader;
        CacheStats m_cacheStats;  // cumulative statistics about this cache
    };

    template <typename K, typename V>
    Future<V> LoadingCache<K, V>::get(const K& key)
    {
        int64_t now = getTick();

        boost::lock_guard<boost::mutex> lock(m_mutex);
        typename M::iterator it = getLiveObj(key, now);
        if (it != m_map.end()) {
            scheduleRefresh(now, it);
            markAccess(it);
            ++m_cacheStats.m_hitCount;
            return it->second.getValue();
        }

        // The value was either absent or expired. Scheduled to load the associated value.
        ++m_cacheStats.m_missCount;
        return load(key, now);
    }

    template <typename K, typename V>
    Future<V> LoadingCache<K, V>::getIfPresent(const K& key)
    {
        int64_t now = getTick();
        boost::lock_guard<boost::mutex> lock(m_mutex);
        typename M::iterator it = getLiveObj(key, now);
        if (it != m_map.end()) {
            scheduleRefresh(now, it);
            markAccess(it);
            return it->second.getValue();
        }
        return Future<V>();
    }

    template <typename K, typename V>
    void LoadingCache<K, V>::put(const K& key, const V& value)
    {
        Object obj(value, getTick());
        boost::lock_guard<boost::mutex> lock(m_mutex);
        typename M::iterator it = m_map.lower_bound(key);
        if (it != m_map.end() && !m_map.key_comp()(key, it->first)) {
            // Found; replace it.
            it->second = obj;
        } else {
            // Not found; insert it.
            it = m_map.insert(it, std::make_pair(key, obj));
        }
        addToFront(it);
        evictEntries();
    }

    template <typename K, typename V>
    void LoadingCache<K, V>::invalidate(const K& key)
    {
        boost::lock_guard<boost::mutex> lock(m_mutex);
        typename M::iterator it = m_map.find(key);
        if (it != m_map.end())
            remove(it);
    }

    template <typename K, typename V>
    void LoadingCache<K, V>::invalidateAll()
    {
        boost::lock_guard<boost::mutex> lock(m_mutex);
        m_map.clear();
        m_listHead = m_map.end();
    }

    template <typename K, typename V>
    typename LoadingCache<K, V>::M::iterator LoadingCache<K, V>::getLiveObj(
            const K& key, int64_t now)
    {
        typename M::iterator it = m_map.find(key);
        if (it != m_map.end()) {
            Object& obj = it->second;
            if (obj.isLoading() || obj.isRefreshing()) {
                return it;
            }
            
            // Check exipration if valid.
            if (obj.isValid()
                    && (m_expireMilliseconds <= 0
                        || now - obj.getWriteTime() < m_expireMilliseconds)) {
                return it;
            }
            
            // Remove invalid or expired value.
            remove(it);
        }
        return m_map.end();  // Expired, invalid or not found.
    }

    template <typename K, typename V>
    void LoadingCache<K, V>::scheduleRefresh(int64_t now, typename M::iterator it)
    {
        BOOST_ASSERT(it != m_map.end());
        Object& obj = it->second;
        if (!obj.isValid() || m_refreshInterval <= 0)
            return;
        if (now - obj.getWriteTime() >= m_refreshInterval || obj.isRefreshFailure()) {
            Future<V> newVal;
            if (m_reloader)
                newVal = m_reloader(it->first, obj.getValue().get());
            else if (m_loader)
                newVal = m_loader(it->first);
            else
                return;
            obj.refresh(newVal, now);
        }
    }

    template <typename K, typename V>
    Future<V> LoadingCache<K, V>::load(const K& key, int64_t now)
    {
        if (!m_loader)
            return Future<V>();

        Object obj;
        obj.newVal = m_loader(key);
        obj.writeTime = now;
        std::pair<typename M::iterator, bool> pair = m_map.insert(std::make_pair(key, obj));
        BOOST_ASSERT(pair.second);
        addToFront(pair.first);
        evictEntries();
        return obj.newVal;
    }

    template <typename K, typename V>
    void LoadingCache<K, V>::remove(typename M::iterator it)
    {
        removeFromAccessList(it);
        m_map.erase(it);
    }

    template <typename K, typename V>
    void LoadingCache<K, V>::evictEntries()
    {
        if (m_capacity > 0) {
            while (m_map.size() > m_capacity) {
                remove(m_listHead->second.prev);
                ++m_cacheStats.m_evictionCount;
            }
        }
    }

    template <typename K, typename V>
    void LoadingCache<K, V>::markAccess(typename M::iterator it)
    {
        removeFromAccessList(it);
        addToFront(it);
    }

    template <typename K, typename V>
    void LoadingCache<K, V>::addToFront(typename M::iterator it)
    {
        if (m_listHead == m_map.end()) {
            // The list is empty.
            m_listHead = it;
            it->second.prev = it->second.next = it;
            return;
        }

        const typename M::iterator& prev = m_listHead->second.prev;
        const typename M::iterator& next = m_listHead;
        it->second.prev = prev;
        prev->second.next = it;
        it->second.next = next;
        next->second.prev = it;
        m_listHead = it;
    }

    template <typename K, typename V>
    void LoadingCache<K, V>::removeFromAccessList(typename M::iterator it)
    {
        if (it->second.next == it) {
            // This item is the only element.
            m_listHead = m_map.end();
            return;
        }

        const typename M::iterator& next = it->second.next;
        const typename M::iterator& prev = it->second.prev;
        prev->second.next = next;
        next->second.prev = prev;
        if (m_listHead == it)
            m_listHead = it->second.next;
    }

}  // namespace wsd

#endif  // WSD_LOADING_CACHE_H
