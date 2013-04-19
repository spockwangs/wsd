// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __COND_H__
#define __COND_H__

#include <pthread.h>
#include "mutex.h"
#include "wsd_magic.h"

namespace wsd {

    class Cond {
    public:
        Cond()
        {
            ::pthread_cond_init(&m_cond, NULL);
        }

        ~Cond()
        {
            ::pthread_cond_destroy(&m_cond);
        }

        void broadcast() const
        {
            ::pthread_cond_broadcast(&m_cond);
        }

        void signal() const
        {
            ::pthread_cond_signal(&m_cond);
        }

        void wait(const Mutex& mutex) const
        {
            ::pthread_cond_wait(&m_cond, &mutex.m_mutex);
        }

    private:
        DISALLOW_COPY_AND_ASSIGN(Cond);
        
        mutable pthread_cond_t m_cond;
    };

}  // namespace wsd

#endif  // __COND_H__
