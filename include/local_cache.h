// Copyright (c) 2013 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//
#ifndef __LOCAL_CACHE_H__
#define __LOCAL_CACHE_H__

#include <sys/time.h>
#include <map>
#include <list>
#include <utility>
#include "boost/shared_ptr.hpp"
#include "boost/exception_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "promise.h"
#include "wsd_magic.h"

namespace wsd {

    template <typename K, typename V>
    class LocalCache {
    public:
        LocalCache(size_t capacity, int refreshMilli = 0)
            : m_capacity(capacity),
              m_refreshMilli(refreshMilli)
        {}

        virtual ~LocalCache() {}
        
        /**
         * Returns the value associated with the key in the cache. If not available it
         * will be loaded and returned.
         *
         * Note: it will never return a NULL boost::shared_ptr<>.
         */
        boost::shared_ptr<const V> get(K key);

        void put(const K& key, const V& value);

        size_t size() const
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            return m_map_storage.size();
        }
        
    private:
        DISALLOW_COPY_AND_ASSIGN(LocalCache);
        
        struct MapValueType;
        typedef std::map<K, MapValueType> map_storage_type;            
        typedef typename map_storage_type::value_type Entry;
        struct CacheValue;
        struct MapValueType {
            MapValueType(const boost::shared_ptr<CacheValue>& cache_value)
                : value_ptr(cache_value),
                  next(0),
                  prev(0)
            {}
            
            boost::shared_ptr<CacheValue> value_ptr;
            typename map_storage_type::value_type *next;
            typename map_storage_type::value_type *prev;
        };

        class CacheValue {
        public:
            CacheValue()
                : m_write_time(0)
            {}
            
            virtual ~CacheValue() {}
            virtual boost::shared_ptr<const V> get() = 0;
            virtual bool isLoading() const = 0;
            virtual bool isValid() const = 0;

            void setWriteTime(long now)
            {
                m_write_time = now;
            }

            long getWriteTime() const
            {
                return m_write_time;
            }
            
        private:
            long m_write_time;
        };

        class LoadedCacheValue;
        class LoadingCacheValue;
        
        class UpdateLoadingCacheValue {
        public:
            UpdateLoadingCacheValue(const boost::shared_ptr<LoadingCacheValue>& loading_cache_value)
                : m_loading_cache_value(loading_cache_value)
            {}

            void run(const Future<V>& future)
            {
                try {
                    const V& v = future.get();
                    m_loading_cache_value->set(boost::shared_ptr<V>(new V(v)));
                    m_loading_cache_value->setWriteTime(getNow());
                } catch (...) {
                    m_loading_cache_value->setException(boost::current_exception());
                }
            }

            boost::shared_ptr<LoadingCacheValue> m_loading_cache_value;
        };

        class AccessQueue {
        public:
            AccessQueue()
                : m_head(0)
            {}

            // Add an entry to the tail of this queue.
            void add(Entry *e);

            void remove(Entry *e);

            Entry *head();
        private:
            Entry *m_head;
        };
        
        virtual V load(K key) = 0;
        virtual Future<V> reload(K key, V oldValue) = 0;
        
        // guarded by 'm_mutex'
        void recordAccess(const typename map_storage_type::iterator& it);

        // guarded by 'm_mutex'
        void evict();
        
        // guarded by 'm_mutex'
        Entry *getNextEvictable();
        
        void removeEntry(Entry *e);
        
        boost::shared_ptr<const V> scheduleRefresh(K key, const boost::shared_ptr<CacheValue>& old_value);
        
        boost::shared_ptr<V> refresh(K key);
        
        boost::shared_ptr<LoadingCacheValue> insertLoadingValue(K key);
        
        Future<V> loadAsync(const K& key,
                            const boost::shared_ptr<LoadingCacheValue>& loading_cache_value);

        static long getNow()
        {
            struct timeval tv;
            int err = gettimeofday(&tv, NULL);
            if (err)
                BOOST_THROW_EXCEPTION(std::runtime_error(""));
            return tv.tv_sec * 1000 + tv.tv_usec/1000;
        }
        
        mutable boost::mutex m_mutex;
        size_t m_capacity;
        map_storage_type m_map_storage;
        AccessQueue m_access_queue;
        int m_refreshMilli;
    };

    template <typename K, typename V>
    class LocalCache<K, V>::LoadedCacheValue : public LocalCache::CacheValue {
    public:
        LoadedCacheValue(const V& v)
            : m_value(new V(v))
        {}
            
        virtual boost::shared_ptr<const V> get()
        {
            return m_value;
        }

        virtual bool isLoading() const
        {
            return false;
        }

        virtual bool isValid() const
        {
            return true;
        }

    private:
        boost::shared_ptr<V> m_value;
    };
        
    template <typename K, typename V>
    class LocalCache<K, V>::LoadingCacheValue : public LocalCache::CacheValue {
    public:
        LoadingCacheValue(const boost::shared_ptr<CacheValue>& old_value = boost::shared_ptr<CacheValue>())
            : m_old_value(old_value)
        {}
        
        virtual boost::shared_ptr<const V> get()
        {
            return m_promise.getFuture().get();
        }

        virtual bool isLoading() const
        {
            return !m_promise.getFuture().isDone();
        }
            
        virtual bool isValid() const
        {
            Future<boost::shared_ptr<V> > future = m_promise.getFuture();
            if (future.isDone() && future.hasException())
                return false;
            return true;
        }
        
        void set(const boost::shared_ptr<V>& v)
        {
            m_promise.setValue(v);
        }

        void setException(const boost::exception_ptr& e)
        {
            m_promise.setException(e);
        }

        boost::shared_ptr<CacheValue> getOldValue() const
        {
            return m_old_value;
        }
            
    private:
        boost::shared_ptr<CacheValue> m_old_value;
        Promise<boost::shared_ptr<V> > m_promise;
    };

    template <typename K, typename V>
    boost::shared_ptr<const V> LocalCache<K, V>::get(K key)
    {
        boost::unique_lock<boost::mutex> lock(m_mutex);
        typename map_storage_type::iterator it = m_map_storage.find(key);
        if (it != m_map_storage.end()) {
            boost::shared_ptr<CacheValue> v = it->second.value_ptr;
            if (v->isValid()) {
                recordAccess(it);
                lock.unlock();
                return scheduleRefresh(key, v);
            }

            removeEntry(&*it);
        }

        // Not found; insert a new one.
        boost::shared_ptr<LoadingCacheValue> loading_cache_value(new LoadingCacheValue());
        it = m_map_storage.insert(std::make_pair(key, MapValueType(loading_cache_value))).first;
        m_access_queue.add(&*it);
        recordAccess(it);
        evict();
        lock.unlock();
        try {
            const V& v = load(key);
            loading_cache_value->set(boost::shared_ptr<V>(new V(v)));
            loading_cache_value->setWriteTime(getNow());
        } catch (...) {
            loading_cache_value->setException(boost::current_exception());
        }
        return loading_cache_value->get();
    }

    template <typename K, typename V>
    boost::shared_ptr<const V> LocalCache<K, V>::scheduleRefresh(K key, const boost::shared_ptr<CacheValue>& old_value)
    {
        long now = getNow();
        if (m_refreshMilli > 0 && (now - old_value->getWriteTime()) > m_refreshMilli
                && !old_value->isLoading()) {
            boost::shared_ptr<const V> new_value = refresh(key);
            if (new_value)
                return new_value;
        }
        return old_value->get();
    }
            
    template <typename K, typename V>
    boost::shared_ptr<V> LocalCache<K, V>::refresh(K key)
    {
        boost::shared_ptr<LoadingCacheValue> loading_cache_value = insertLoadingValue(key);
        if (!loading_cache_value)
            return boost::shared_ptr<V>();

        Future<V> result = loadAsync(key, loading_cache_value);
        if (result.isDone()) {
            try {
                return boost::shared_ptr<V>(new V(result.get()));
            } catch (...) {
            }
        }
        return boost::shared_ptr<V>();
    }

    template <typename K, typename V>
    boost::shared_ptr<typename LocalCache<K, V>::LoadingCacheValue> LocalCache<K, V>::insertLoadingValue(K key)
    {
        boost::lock_guard<boost::mutex> lock(m_mutex);
        typename map_storage_type::iterator it = m_map_storage.find(key);
        if (it != m_map_storage.end()) {
            // Found an existing entry.

            const boost::shared_ptr<CacheValue>& value = it->second.value_ptr;
            if (value->isLoading())
                return boost::shared_ptr<typename LocalCache<K, V>::LoadingCacheValue>();

            boost::shared_ptr<LoadingCacheValue> loading_cache_value(new LoadingCacheValue(value));
            it->second.value_ptr = loading_cache_value;
            return loading_cache_value;
        }

        boost::shared_ptr<LoadingCacheValue> loading_cache_value(new LoadingCacheValue());
        m_map_storage.insert(std::make_pair(key, MapValueType(loading_cache_value)));
        return loading_cache_value;
    }
    
    template <typename K, typename V>
    Future<V> LocalCache<K, V>::loadAsync(const K& key,
                                          const boost::shared_ptr<LoadingCacheValue>& loading_cache_value)
    {
        const boost::shared_ptr<CacheValue>& old_value = loading_cache_value->getOldValue();
        Future<V> future;
        if (old_value) {
            future = reload(key, *old_value->get());
        } else {
            const V& v = load(key);
            future = makeFuture<V>(v);
        }
        future.then(bind(&UpdateLoadingCacheValue::run, owned(new UpdateLoadingCacheValue(loading_cache_value))));
        return future;
    }
    
    template <typename K, typename V>
    void LocalCache<K, V>::put(const K& key, const V& value)
    {
        boost::lock_guard<boost::mutex> lock(m_mutex);
        typename map_storage_type::iterator it = m_map_storage.find(key);
        if (it != m_map_storage.end()) {
            recordAccess(it);
            it->second.value_ptr.reset(new LoadedCacheValue(value));
            it->second.value_ptr->setWriteTime(getNow());
            evict();
            return;
        }

        // Not found; insert a new one.
        boost::shared_ptr<CacheValue> loaded_cache_value(new LoadedCacheValue(value));
        loaded_cache_value->setWriteTime(getNow());
        it = m_map_storage.insert(std::make_pair(key, MapValueType(loaded_cache_value))).first;
        m_access_queue.add(&*it);
        recordAccess(it);
        evict();
    }
    
    template <typename K, typename V>
    void LocalCache<K, V>::AccessQueue::add(Entry *e)
    {
        // this is the first element to add
        if (!m_head) {
            m_head = e;
            e->second.prev = e;
            e->second.next = e;
            return;
        }

        Entry *prev = e->second.prev;
        Entry *next = e->second.next;
        if (prev && next) {
            // unlink
            prev->second.next = next;
            next->second.prev = prev;
        }

        // add to tail
        m_head->second.prev->second.next = e;
        e->second.prev = m_head->second.prev;
        e->second.next = m_head;
        m_head->second.prev = e;
    }
    
    template <typename K, typename V>
    void LocalCache<K, V>::AccessQueue::remove(Entry *e)
    {
        Entry *prev = e->second.prev;
        Entry *next = e->second.next;
        BOOST_ASSERT(prev && next);

        if (m_head == e) {
            if (m_head->second.next == e) {
                // this is the only one element
                m_head = 0;
                return;
            }
            m_head = e->second.next;
        }

        // unlink
        prev->second.next = next;
        next->second.prev = prev;
    }
    
    template <typename K, typename V>
    typename LocalCache<K, V>::Entry *LocalCache<K, V>::AccessQueue::head()
    {
        return m_head;
    }

    template <typename K, typename V>
    void LocalCache<K, V>::recordAccess(const typename map_storage_type::iterator& it)
    {
        m_access_queue.remove(&*it);
        m_access_queue.add(&*it);
    }

    template <typename K, typename V>
    void LocalCache<K, V>::evict()
    {
        while (m_map_storage.size() > m_capacity) {
            Entry *e = getNextEvictable();
            if (e)
                removeEntry(e);
        }
    }

    template <typename K, typename V>
    typename LocalCache<K, V>::Entry *LocalCache<K, V>::getNextEvictable()
    {
        return m_access_queue.head();
    }

    template <typename K, typename V>
    void LocalCache<K, V>::removeEntry(Entry *e)
    {
        BOOST_ASSERT(e);

        m_access_queue.remove(e);
        size_t n = m_map_storage.erase(e->first);
        BOOST_ASSERT(n == 1);
    }

}  // namespace wsd

#endif  // __LOCAL_CACHE_H__
