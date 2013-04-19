// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <pthread.h>
#include "wsd_magic.h"
#include "wsd_assert.h"
#include "wsd_exception.h"
#include "lock.h"

namespace wsd {

    class Mutex {
    public:
        typedef LockT<Mutex> Lock;
        
        Mutex()
        {
            int err = ::pthread_mutex_init(&m_mutex, NULL);
            if (err)
                throw SyscallException(__FILE__, __LINE__, err);
        }

        ~Mutex()
        {
            int err = ::pthread_mutex_destroy(&m_mutex);
            WSD_ASSERT(err == 0);
        }

        void lock() const
        {
            int err = ::pthread_mutex_lock(&m_mutex);
            if (err)
                throw SyscallException(__FILE__, __LINE__, err);
        }

        void unlock() const
        {
            int err = ::pthread_mutex_unlock(&m_mutex);
            if (err)
                throw SyscallException(__FILE__, __LINE__, err);
        }

    private:
        DISALLOW_COPY_AND_ASSIGN(Mutex);

        mutable pthread_mutex_t m_mutex;

        friend class Cond;
    };
    
}  // namespace wsd

#endif  // __MUTEX_H__
