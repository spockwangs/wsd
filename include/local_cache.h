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
#include "shared_ptr.h"
#include "mutex.h"
#include "exception_ptr.h"
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
         * Note: it will never return a NULL SharedPtr<>.
         */
        SharedPtr<const V> get(K key);

        void put(const K& key, const V& value);

        size_t size() const
        {
            Mutex::Lock lock(m_mutex);
            return m_map_storage.size();
        }
        
    private:
        DISALLOW_COPY_AND_ASSIGN(LocalCache);
        
        struct MapValueType;
        typedef std::map<K, MapValueType> map_storage_type;            
        typedef typename map_storage_type::value_type Entry;
        struct CacheValue;
        struct MapValueType {
            MapValueType(const SharedPtr<CacheValue>& cache_value)
                : value_ptr(cache_value),
                  next(0),
                  prev(0)
            {}
            
            SharedPtr<CacheValue> value_ptr;
            typename map_storage_type::value_type *next;
            typename map_storage_type::value_type *prev;
        };

        class CacheValue {
        public:
            CacheValue()
                : m_write_time(0)
            {}
            
            virtual ~CacheValue() {}
            virtual SharedPtr<const V> get() = 0;
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
            UpdateLoadingCacheValue(const SharedPtr<LoadingCacheValue>& loading_cache_value)
                : m_loading_cache_value(loading_cache_value)
            {}

            void run(const Future<V>& future)
            {
                try {
                    const V& v = future.get();
                    m_loading_cache_value->set(SharedPtr<V>(new V(v)));
                    m_loading_cache_value->setWriteTime(getNow());
                } catch (...) {
                    m_loading_cache_value->setException(currentException());
                }
            }

            SharedPtr<LoadingCacheValue> m_loading_cache_value;
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
        
        SharedPtr<const V> scheduleRefresh(K key, const SharedPtr<CacheValue>& old_value);
        
        SharedPtr<V> refresh(K key);
        
        SharedPtr<LoadingCacheValue> insertLoadingValue(K key);
        
        Future<V> loadAsync(const K& key,
                            const SharedPtr<LoadingCacheValue>& loading_cache_value);

        static long getNow()
        {
            struct timeval tv;
            int err = gettimeofday(&tv, NULL);
            if (err)
                throw SyscallException(__FILE__, __LINE__, err);
            return tv.tv_sec * 1000 + tv.tv_usec/1000;
        }
        
        Mutex m_mutex;
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
            
        virtual SharedPtr<const V> get()
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
        SharedPtr<V> m_value;
    };
        
    template <typename K, typename V>
    class LocalCache<K, V>::LoadingCacheValue : public LocalCache::CacheValue {
    public:
        LoadingCacheValue(const SharedPtr<CacheValue>& old_value = SharedPtr<CacheValue>())
            : m_old_value(old_value)
        {}
        
        virtual SharedPtr<const V> get()
        {
            return m_promise.getFuture().get();
        }

        virtual bool isLoading() const
        {
            return !m_promise.getFuture().isDone();
        }
            
        virtual bool isValid() const
        {
            Future<SharedPtr<V> > future = m_promise.getFuture();
            if (future.isDone() && future.hasException())
                return false;
            return true;
        }
        
        void set(const SharedPtr<V>& v)
        {
            m_promise.setValue(v);
        }

        void setException(const ExceptionPtr& e)
        {
            m_promise.setException(e);
        }

        SharedPtr<CacheValue> getOldValue() const
        {
            return m_old_value;
        }
            
    private:
        SharedPtr<CacheValue> m_old_value;
        Promise<SharedPtr<V> > m_promise;
    };

    template <typename K, typename V>
    SharedPtr<const V> LocalCache<K, V>::get(K key)
    {
        Mutex::Lock lock(m_mutex);
        typename map_storage_type::iterator it = m_map_storage.find(key);
        if (it != m_map_storage.end()) {
            SharedPtr<CacheValue> v = it->second.value_ptr;
            if (v->isValid()) {
                recordAccess(it);
                lock.release();
                return scheduleRefresh(key, v);
            }

            removeEntry(&*it);
        }

        // Not found; insert a new one.
        SharedPtr<LoadingCacheValue> loading_cache_value(new LoadingCacheValue());
        it = m_map_storage.insert(std::make_pair(key, MapValueType(loading_cache_value))).first;
        m_access_queue.add(&*it);
        recordAccess(it);
        evict();
        lock.release();
        try {
            const V& v = load(key);
            loading_cache_value->set(SharedPtr<V>(new V(v)));
            loading_cache_value->setWriteTime(getNow());
        } catch (...) {
            loading_cache_value->setException(currentException());
        }
        return loading_cache_value->get();
    }

    template <typename K, typename V>
    SharedPtr<const V> LocalCache<K, V>::scheduleRefresh(K key, const SharedPtr<CacheValue>& old_value)
    {
        long now = getNow();
        if (m_refreshMilli > 0 && (now - old_value->getWriteTime()) > m_refreshMilli
                && !old_value->isLoading()) {
            SharedPtr<const V> new_value = refresh(key);
            if (new_value)
                return new_value;
        }
        return old_value->get();
    }
            
    template <typename K, typename V>
    SharedPtr<V> LocalCache<K, V>::refresh(K key)
    {
        SharedPtr<LoadingCacheValue> loading_cache_value = insertLoadingValue(key);
        if (!loading_cache_value)
            return SharedPtr<V>();

        Future<V> result = loadAsync(key, loading_cache_value);
        if (result.isDone()) {
            try {
                return SharedPtr<V>(new V(result.get()));
            } catch (...) {
            }
        }
        return SharedPtr<V>();
    }

    template <typename K, typename V>
    SharedPtr<typename LocalCache<K, V>::LoadingCacheValue> LocalCache<K, V>::insertLoadingValue(K key)
    {
        Mutex::Lock lock(m_mutex);
        typename map_storage_type::iterator it = m_map_storage.find(key);
        if (it != m_map_storage.end()) {
            // Found an existing entry.

            const SharedPtr<CacheValue>& value = it->second.value_ptr;
            if (value->isLoading())
                return SharedPtr<typename LocalCache<K, V>::LoadingCacheValue>();

            SharedPtr<LoadingCacheValue> loading_cache_value(new LoadingCacheValue(value));
            it->second.value_ptr = loading_cache_value;
            return loading_cache_value;
        }

        SharedPtr<LoadingCacheValue> loading_cache_value(new LoadingCacheValue());
        m_map_storage.insert(std::make_pair(key, MapValueType(loading_cache_value)));
        return loading_cache_value;
    }
    
    template <typename K, typename V>
    Future<V> LocalCache<K, V>::loadAsync(const K& key,
                                          const SharedPtr<LoadingCacheValue>& loading_cache_value)
    {
        const SharedPtr<CacheValue>& old_value = loading_cache_value->getOldValue();
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
        Mutex::Lock lock(m_mutex);
        typename map_storage_type::iterator it = m_map_storage.find(key);
        if (it != m_map_storage.end()) {
            recordAccess(it);
            it->second.value_ptr.reset(new LoadedCacheValue(value));
            it->second.value_ptr->setWriteTime(getNow());
            evict();
            return;
        }

        // Not found; insert a new one.
        SharedPtr<CacheValue> loaded_cache_value(new LoadedCacheValue(value));
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
        WSD_ASSERT(prev && next);

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
        WSD_ASSERT(e);

        m_access_queue.remove(e);
        size_t n = m_map_storage.erase(e->first);
        WSD_ASSERT(n == 1);
    }

}  // namespace wsd

#endif  // __LOCAL_CACHE_H__
