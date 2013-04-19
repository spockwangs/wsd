// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __LOCKER_H__
#define __LOCKER_H__

#include "wsd_assert.h"
#include "wsd_magic.h"

namespace wsd {

    template <typename M>
    class LockT {
    public:
        LockT(const M& mutex)
            : m_mutex(mutex),
              m_acquired(false)
        {
            m_mutex.lock();
            m_acquired = true;
        }

        ~LockT()
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
        DISALLOW_COPY_AND_ASSIGN(LockT);
        
        const M& m_mutex;
        mutable bool m_acquired;
    };
    
}  // namespace wsd

#endif  // __LOCKER_H__
