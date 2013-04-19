// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __CONCURRENT_MAP_H__
#define __CONCURRENT_MAP_H__

#include <map>
#include <utility>
#include <iterator>
#include "shared_ptr.h"
#include "scoped_ptr.h"
#include "lock.h"
#include "rw_mutex.h"

namespace wsd {

    template <typename K, typename V, typename Cmp>
    class ConcurrentMap;

    template <typename K, typename V, typename Cmp>
    bool operator==(const ConcurrentMap<K, V, Cmp>& lhs, const ConcurrentMap<K, V, Cmp>& rhs);

    template <typename K, typename V, typename Cmp = std::less<K> >
    class ConcurrentMap {
    private:
        struct Value;
            
        typedef std::map<K, SharedPtr<Value>, Cmp> map_type;

        // Concurrent operations may not be applied on this map when traversing.
        class ConstIterator;
        class Iterator;

    public:
        typedef K key_type;
        typedef V mapped_type;
        typedef std::pair<const K, V> value_type;
        typedef typename map_type::size_type size_type;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef Iterator iterator;
        typedef ConstIterator const_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

        ConcurrentMap()
            : m_rw_mutex(new RwMutex)
        {}
        
        template <typename InputIterator>
        ConcurrentMap(InputIterator first, InputIterator last)
            : m_rw_mutex(new RwMutex)
        {
            for (; first != last; ++first)
                m_map.insert(std::make_pair(first->first, SharedPtr<Value>(new Value(*first))));
        }
        
        /**
         * Copies a map. The map being copied may have concurrent operations on it.
         */
        ConcurrentMap(const ConcurrentMap& o)
            : m_rw_mutex(new RwMutex)
        {
            RwMutex::ReadLock rlock(*o.m_rw_mutex);
            for (typename map_type::const_iterator it = o.m_map.begin(); it != o.m_map.end(); ++it)
                m_map.insert(m_map.end(), std::make_pair(it->first, SharedPtr<Value>(new Value(it->second->value))));
        }
        
        /**
         * Copy all key-value pairs from 'o' to this map.
         */
        ConcurrentMap& operator=(const ConcurrentMap& o)
        {
            if (this != &o) {
                ConcurrentMap tmp(o);

                // Lock on 'tmp' is not necessary because no one other than this thread
                // can access 'tmp'.
                RwMutex::WriteLock wlock(*m_rw_mutex);
                m_map.swap(tmp.m_map);
            }
            return *this;
        }
        
        // concurrent access
        class Accessor;
        class ConstAccessor;

        void swap(ConcurrentMap& o)
        {
            if (this == &o)
                return;

            // Lock in a consistent order to avoid deadlock.
            ConcurrentMap *lhs = NULL, *rhs = NULL;
            if (this < &o) {
                lhs = this;
                rhs = &o;
            } else {
                lhs = &o;
                rhs = this;
            }
            
            RwMutex::WriteLock wlock1(*lhs->m_rw_mutex);
            RwMutex::WriteLock wlock2(*rhs->m_rw_mutex);
            m_map.swap(o.m_map);
        }
        
        /**
         * Returns true if the map is empty.
         *
         * \throws nothing
         */
        bool empty() const
        {
            RwMutex::ReadLock rlock(*m_rw_mutex);
            return m_map.empty();
        }
        
        /**
         * Returns the number of key-value pairs in the map.
         *
         * \throws nothing
         */
        size_t size() const
        {
            RwMutex::ReadLock rlock(*m_rw_mutex);
            return m_map.size();
        }

        /**
         * Erased all pairs from the map. If an pair is pointed by an accessor, the pair
         * is nonethese erased but the accessor is still pointing to it.
         *
         * \throws nothing
         */
        void clear()
        {
            RwMutex::WriteLock wlock(*m_rw_mutex);
            m_map.clear();
        }
        
        /**
         * Returns 1 if the map contains the specified key, or 0 otherwise.
         */
        size_t count(const key_type& key) const
        {
            RwMutex::ReadLock rlock(*m_rw_mutex);
            return m_map.count(key);
        }
        
        /**
         * Searches for the pair with the given key. If key is found, sets
         * 'const_accessor' to provide read-only access to the matching pair.
         *
         * \returns true if the key is found 
         * \throws nothing
         */
        bool find(const key_type& key, ConstAccessor *const_accessor) const;

        /**
         * Searches for the pair with the given key. If key is found, sets 'accessor' to
         * provide write access to the matching pair.
         *
         * \returns true if the key is found
         * \throws nothing
         */
        bool find(const key_type& key, Accessor *accessor);

        /**
         * Insert a new pair copy-constructed from 'value' into the map if the given key
         * is not present.
         *
         * If an exception is thrown, the map is intact.
         *
         * \returns true if a new pair is inserted
         */
        bool insert(const value_type& value);

        /**
         * Inserts a new pair copy-constructed from 'value' into the map if the given key
         * is not present. Sets 'const_accessor' to provide read-only access to the
         * matching pair.
         *
         * If an exception is thrown, the map is intact.
         *
         * \returns true if a new pair is inserted.
         */
        bool insert(const value_type& value, ConstAccessor *const_accessor);

        /**
         * Inserts a new 'pair(key, V())' into the map if the given key is not
         * present. Sets 'const_accessor' to provide read-only access to the matching
         * pair.
         *
         * If an exception is thrown, the map is intact.
         *
         * \returns true if a new pair is inserted.
         */
        bool insert(const key_type& key, ConstAccessor *const_accessor);

        /**
         * Inserts a new pair copy-constructed from 'value' into the map if the given key
         * is not present. Sets 'accessor' to provide read-only access to the
         * matching pair.
         *
         * If an exception is thrown, the map is intact.
         *
         * \returns true if a new pair is inserted.
         */
        bool insert(const value_type& value, Accessor *accessor);

        /**
         * Inserts a new 'pair(key, V())' into the map if the given key is not
         * present. Sets 'const_accessor' to provide write access to the matching
         * pair.
         *
         * If an exception is thrown, the map is intact.
         *
         * \returns true if a new pair is inserted.
         */
        bool insert(const key_type& value, Accessor *accessor);

        /**
         * Searches for the pair with the given key. Removies the matching pair if it
         * exists. If there is an accessor pointing to the pair, the pair is nonetheless
         * removed but the accessor can still access it.
         *
         * \returns true if the pair is removed by this call, or false if the key was not found 
         * \throws nothing
         */
        bool erase(const key_type& key);

        /**
         * Removes the pair referenced by 'const_accessor'.
         *
         * \pre const_accessor.empty() == false
         * \returns true if the pair was removed by this thread, or false if the pair was
         * removed by another thread.
         */
        bool erase(ConstAccessor *const_accessor);

        /**
         * Removes the pair referenced by 'accessor'.
         *
         * \pre accessor.empty() == false
         * \returns true if the pair was removed by this thread, or false if the pair was
         * removed by another thread.
         */
        bool erase(Accessor *accessor);

        const_iterator begin() const
        {
            return ConstIterator(m_map.begin());
        }
        
        const_iterator end() const
        {
            return ConstIterator(m_map.end());
        }

        iterator begin()
        {
            return Iterator(m_map.begin());
        }

        iterator end()
        {
            return Iterator(m_map.end());
        }
        
    private:
        ScopedPtr<RwMutex> m_rw_mutex;
        map_type m_map;

        friend bool operator==<>(const ConcurrentMap& lhs, const ConcurrentMap& rhs);
    };

    template <typename K, typename V, typename Cmp>
    struct ConcurrentMap<K, V, Cmp>::Value {
        Value(const value_type& v)
            : value(v)
        {}
        
        RwMutex rw_mutex;
        value_type value;
    };

    template <typename K, typename V, typename Cmp>
    class ConcurrentMap<K, V, Cmp>::ConstAccessor {
    public:
        virtual ~ConstAccessor()
        {
            release();
        }
        
        bool empty() const
        {
            return !m_value;
        }
        
        const_reference operator*() const
        {
            WSD_ASSERT(m_value);
            return m_value->value;
        }
        
        const_pointer operator->() const
        {
            WSD_ASSERT(m_value);
            return &m_value->value;
        }
        
        void release()
        {
            if (!m_value)
                return;
            m_value->rw_mutex.unlock();
            m_value.reset();
        }
        
    protected:

        // invariant: if m_value is true then we have acquired the lock on it.
        SharedPtr<Value> m_value;

    private:
        void acquire(const SharedPtr<Value>& o)
        {
            WSD_ASSERT(o);
            if (m_value != o) {
                release();
                m_value = o;
                m_value->rw_mutex.readLock();
            }
        }
        
        friend class ConcurrentMap;
    };
    
    template <typename K, typename V, typename Cmp>
    class ConcurrentMap<K, V, Cmp>::Accessor : public ConcurrentMap<K, V, Cmp>::ConstAccessor {
    public:
        value_type& operator*() const
        {
            WSD_ASSERT(this->m_value);
            return this->m_value->value;
        }
        
        value_type* operator->() const
        {
            WSD_ASSERT(this->m_value);
            return &this->m_value->value;
        }

    private:
        void acquire(const SharedPtr<Value>& o)
        {
            WSD_ASSERT(o);
            if (this->m_value != o) {
                this->release();
                this->m_value = o;
                this->m_value->rw_mutex.writeLock();
            }
        }

        friend class ConcurrentMap;
    };
    
    template <typename K, typename V, typename Cmp>
    bool ConcurrentMap<K, V, Cmp>::find(const key_type& key, ConstAccessor *const_accessor) const
    {
        WSD_ASSERT(const_accessor != NULL);
        
        const_accessor->release();
        RwMutex::ReadLock rlock(*m_rw_mutex);
        typename map_type::const_iterator it = m_map.find(key);
        if (it == m_map.end())
            return false;

        const_accessor->acquire(it->second);
        return true;
    }
    
    template <typename K, typename V, typename Cmp>
    bool ConcurrentMap<K, V, Cmp>::find(const key_type& key, Accessor *accessor)
    {
        WSD_ASSERT(accessor != NULL);
        
        accessor->release();
        RwMutex::ReadLock rlock(*m_rw_mutex);
        typename map_type::const_iterator it = m_map.find(key);
        if (it == m_map.end())
            return false;

        accessor->acquire(it->second);
        return true;
    }
    
    template <typename K, typename V, typename Cmp>
    bool ConcurrentMap<K, V, Cmp>::insert(const value_type& value)
    {
        RwMutex::WriteLock wlock(*m_rw_mutex);
        return m_map.insert(std::make_pair(value.first, SharedPtr<Value>(new Value(value)))).second;
    }

    template <typename K, typename V, typename Cmp>
    bool ConcurrentMap<K, V, Cmp>::insert(const value_type& value, ConstAccessor *const_accessor)
    {
        WSD_ASSERT(const_accessor != NULL);
        RwMutex::WriteLock wlock(*m_rw_mutex);
        const std::pair<typename map_type::iterator, bool>& r =
            m_map.insert(std::make_pair(value.first, SharedPtr<Value>(new Value(value))));
        const_accessor->acquire(r.first->second);
        return r.second;
    }
        
    template <typename K, typename V, typename Cmp>
    bool ConcurrentMap<K, V, Cmp>::insert(const key_type& key, ConstAccessor *const_accessor)
    {
        return insert(std::make_pair(key, V()), const_accessor);
    }
    
    template <typename K, typename V, typename Cmp>
    bool ConcurrentMap<K, V, Cmp>::insert(const value_type& value, Accessor *accessor)
    {
        WSD_ASSERT(accessor != NULL);
        RwMutex::WriteLock wlock(*m_rw_mutex);
        const std::pair<typename map_type::iterator, bool>& r =
            m_map.insert(std::make_pair(value.first, SharedPtr<Value>(new Value(value))));
        accessor->acquire(r.first->second);
        return r.second;
    }
        
    template <typename K, typename V, typename Cmp>
    bool ConcurrentMap<K, V, Cmp>::insert(const key_type& key, Accessor *accessor)
    {
        return insert(std::make_pair(key, V()), accessor);
    }
    
    template <typename K, typename V, typename Cmp>
    bool ConcurrentMap<K, V, Cmp>::erase(const key_type& key)
    {
        RwMutex::WriteLock wlock(*m_rw_mutex);
        return m_map.erase(key) > 0;
    }

    template <typename K, typename V, typename Cmp>
    bool ConcurrentMap<K, V, Cmp>::erase(ConstAccessor *const_accessor)
    {
        WSD_ASSERT(const_accessor);
        WSD_ASSERT(!const_accessor->empty());
        
        const key_type& key = const_accessor->m_value->value.first;
        const_accessor->release();
        return erase(key);
    }
        
    template <typename K, typename V, typename Cmp>
    bool ConcurrentMap<K, V, Cmp>::erase(Accessor *accessor)
    {
        WSD_ASSERT(accessor);
        WSD_ASSERT(!accessor->empty());
        
        const key_type& key = accessor->m_value->value.first;
        accessor->release();
        return erase(key);
    }
        
    template <typename K, typename V, typename Cmp>
    bool operator==(const ConcurrentMap<K, V, Cmp>& lhs, const ConcurrentMap<K, V, Cmp>& rhs)
    {
        if (&lhs == &rhs)
            return true;

        // Lock in a consistent order to avoid deadlock.
        const ConcurrentMap<K, V, Cmp> *l = NULL, *r = NULL;
        if (&lhs < &rhs) {
            l = &lhs;
            r = &rhs;
        } else {
            l = &rhs;
            r = &lhs;
        }
            
        RwMutex::ReadLock rlock1(*l->m_rw_mutex);
        RwMutex::ReadLock rlock2(*r->m_rw_mutex);
        return lhs.m_map == rhs.m_map;
    }
    
    template <typename K, typename V, typename Cmp>
    class ConcurrentMap<K, V, Cmp>::ConstIterator : public std::bidirectional_iterator_tag {
    public:
        typedef typename map_type::const_iterator::difference_type difference_type;
        typedef typename ConcurrentMap::value_type value_type;
        typedef typename ConcurrentMap::pointer pointer;
        typedef typename ConcurrentMap::reference reference;
        typedef std::bidirectional_iterator_tag iterator_category;
        
        ConstIterator() {}

        const_reference operator*() const
        {
            return m_ci->second->value;
        }
        
        const_pointer operator->() const
        {
            return &operator*();
        }
        
        ConstIterator& operator++()
        {
            ++m_ci;
            return *this;
        }
        
        ConstIterator& operator++(int)
        {
            ConstIterator before = *this;
            ++m_ci;
            return before;
        }
        
        ConstIterator& operator--()
        {
            --m_ci;
            return *this;
        }
        
        ConstIterator& operator--(int)
        {
            ConstIterator before = *this;
            --m_ci;
            return before;
        }

    private:
        ConstIterator(const typename map_type::const_iterator& ci)
            : m_ci(ci)
        {}

        typename map_type::const_iterator m_ci;

        friend class ConcurrentMap;
        friend class Iterator;

        friend bool operator==(const ConstIterator& lhs, const ConstIterator& rhs)
        {
            return lhs.m_ci == rhs.m_ci;
        }
        
        friend bool operator!=(const ConstIterator& lhs, const ConstIterator& rhs)
        {
            return lhs.m_ci != rhs.m_ci;
        }
    };
    
    template <typename K, typename V, typename Cmp>
    class ConcurrentMap<K, V, Cmp>::Iterator : public std::bidirectional_iterator_tag {
    public:
        typedef typename map_type::const_iterator::difference_type difference_type;
        typedef typename ConcurrentMap::value_type value_type;
        typedef typename ConcurrentMap::pointer pointer;
        typedef typename ConcurrentMap::reference reference;
        typedef std::bidirectional_iterator_tag iterator_category;
        
        Iterator() {}

        reference operator*() const
        {
            return m_it->second->value;
        }
        
        pointer operator->() const
        {
            return &operator*();
        }
        
        Iterator& operator++()
        {
            ++m_it;
            return *this;
        }
        
        Iterator& operator++(int)
        {
            Iterator before = *this;
            ++m_it;
            return before;
        }
        
        Iterator& operator--()
        {
            --m_it;
            return *this;
        }
        
        Iterator& operator--(int)
        {
            Iterator before = *this;
            --m_it;
            return before;
        }

        operator ConstIterator() const
        {
            return ConstIterator(m_it);
        }
        
    private:
        Iterator(const typename map_type::iterator& it)
            : m_it(it)
        {}

        typename map_type::iterator m_it;

        friend class ConcurrentMap;

        friend bool operator==(const Iterator& lhs, const Iterator& rhs)
        {
            return lhs.m_it == rhs.m_it;
        }
        
        friend bool operator!=(const Iterator& lhs, const Iterator& rhs)
        {
            return lhs.m_it != rhs.m_it;
        }
    };

}  // namespace wsd

#endif  // __CONCURRENT_MAP_H__
