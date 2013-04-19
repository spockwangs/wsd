// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __SHARED_COUNT_BASE_H__
#define __SHARED_COUNT_BASE_H__

#include "atomic.h"
#include "wsd_magic.h"

namespace wsd {

    namespace detail {

        template <typename AtomicPolicy = Atomic>
        class shared_count_base {
        public:

            shared_count_base()
                : m_use_count(1)
            {}

            virtual ~shared_count_base()
            {}
            
            int use_count() const
            {
                return m_use_count.get();
            }

            void increment()
            {
                m_use_count.increment();
            }

            void release()
            {
                if (m_use_count.decrementAndTestZero()) {
                    dispose();
                    delete this;
                }
            }

            // dispose() is called when m_use_count drops to zero, to release resources
            // managed by this object.
            virtual void dispose() = 0;  // never throws
            
        private:

            DISALLOW_COPY_AND_ASSIGN(shared_count_base);
            
            AtomicPolicy m_use_count;
        };

    }  // namespace detail

}  // namespace wsd

#endif  // __SHARED_COUNT_BASE_H__
