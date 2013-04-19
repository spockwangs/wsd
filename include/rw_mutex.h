// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __RW_MUTEX_H__
#define __RW_MUTEX_H__

#include <pthread.h>
#include "wsd_exception.h"
#include "wsd_assert.h"

namespace wsd {

    template <typename M>
    class ReadLockT {
    public:
        ReadLockT(const M& m)
            : m_mutex(m)
        {
            m_mutex.readLock();
            m_acquired = true;
        }

        ~ReadLockT()
        {
            if (m_acquired)
                m_mutex.unlock();
            m_acquired = false;
        }

        void release() const
        {
            WSD_ASSERT(m_acquired);
            m_mutex.unlock();
            m_acquired = false;
        }

        bool acquired() const
        {
            return m_acquired;
        }
        
    private:
        DISALLOW_COPY_AND_ASSIGN(ReadLockT);
        
        const M& m_mutex;
        mutable bool m_acquired;
    };
    
    template <typename M>
    class WriteLockT {
    public:
        WriteLockT(const M& m)
            : m_mutex(m)
        {
            m_mutex.writeLock();
            m_acquired = true;
        }

        ~WriteLockT()
        {
            if (m_acquired)
                m_mutex.unlock();
            m_acquired = false;
        }

        void release() const
        {
            WSD_ASSERT(m_acquired);
            m_mutex.unlock();
            m_acquired = false;
        }

        bool acquired() const
        {
            return m_acquired;
        }
        
    private:
        DISALLOW_COPY_AND_ASSIGN(WriteLockT);
        
        const M& m_mutex;
        mutable bool m_acquired;
    };
    
    class RwMutex {
    public:

        typedef ReadLockT<RwMutex> ReadLock;
        typedef WriteLockT<RwMutex> WriteLock;
        
        RwMutex()
        {
            int err = pthread_rwlock_init(&m_rwlock, NULL);
            if (err)
                throw SyscallException(__FILE__, __LINE__, err);
        }
        
        ~RwMutex()
        {
            int err = pthread_rwlock_destroy(&m_rwlock);
            WSD_ASSERT(err == 0);
        }

        void readLock() const
        {
            int err = pthread_rwlock_rdlock(&m_rwlock);
            if (err)
                throw SyscallException(__FILE__, __LINE__, err);
        }
        
        void writeLock() const
        {
            int err = pthread_rwlock_wrlock(&m_rwlock);
            if (err)
                throw SyscallException(__FILE__, __LINE__, err);
        }
        
        void unlock() const
        {
            int err = pthread_rwlock_unlock(&m_rwlock);
            WSD_ASSERT(err == 0);
        }

    private:
        DISALLOW_COPY_AND_ASSIGN(RwMutex);
        
        mutable pthread_rwlock_t m_rwlock;
    };

}  // namespace wsd

#endif  // __RW_MUTEX_H__
