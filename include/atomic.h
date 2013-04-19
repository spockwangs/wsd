#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#include <pthread.h>
#include "wsd_magic.h"
#include "wsd_assert.h"

namespace wsd {

    namespace detail {
        //
        // Implements the AtomicCountPolicy used by wsd::SharedPtr.
        // 
        class Atomic {
        public:
            Atomic(int n = 0)
                : m_count(n)
            {
                int err = ::pthread_mutex_init(&m_mutex, NULL);
                WSD_ASSERT(err == 0);
            }

            ~Atomic()
            {
                int err = ::pthread_mutex_destroy(&m_mutex);
                WSD_ASSERT(err == 0);
            }

            int get() const
            {
                int n;
                lock();
                n = m_count;
                unlock();
                return n;
            }
            
            void increment()
            {
                lock();
                ++m_count;
                unlock();
            }
            
            bool decrementAndTestZero()
            {
                bool is_zero;
                lock();
                if (--m_count == 0)
                    is_zero = true;
                else
                    is_zero = false;
                unlock();
                return is_zero;
            }

        private:
            DISALLOW_COPY_AND_ASSIGN(Atomic);

            void lock() const
            {
                int err = ::pthread_mutex_lock(&m_mutex);
                WSD_ASSERT(err == 0);
            }

            void unlock() const
            {
                int err = ::pthread_mutex_unlock(&m_mutex);
                WSD_ASSERT(err == 0);
            }
            
            int m_count;
            mutable pthread_mutex_t m_mutex;
        };
    } // namespace detail
}     // namespace wsd

#endif
