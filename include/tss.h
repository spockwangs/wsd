// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __TSS_H__
#define __TSS_H__

#include <pthread.h>
#include <memory>
#include "wsd_exception.h"
#include "shared_ptr.h"
#include <iostream>

namespace wsd {
    
    // ThreadSpecificPtr<T> is a proxy to manage the thread-specific storage which holds a
    // object of T.
    //
    // It can't be copied and assigned.
    //
    // MT-safety: yes.
    template <typename T>
    class ThreadSpecificPtr {
    public:

        typedef T* (ThreadSpecificPtr::*unspecified_bool_type)() const;
        
        // Requires: "delete this->get()" does not throw an exception.
        // Effects: Construct a ThreadSpecificPtr object for storing a pointer to an
        //     object of type T specific to each thread. The delete expression will be used
        //     to destroy any thread-specific object when "reset()" is called, or thread
        //     exits.
        // Throws: wsd::SyscallException if thread-specific storage could not be allocated.
        ThreadSpecificPtr()
            : m_key(0),
              m_cleanup(delete_data)
        {
            int err = pthread_key_create(&m_key, m_cleanup);
            if (err)
                throw SyscallException(__FILE__, __LINE__, err);
        }

        // Requires: "cleanup" is NULL, or "cleanup(this->get())" must not throw an exception.
        // Effects: Construct a ThreadSpecificPtr object for storing a pointer to an
        //     object of type T specific to each thread. If cleanup != NULL, the supplied "cleanup" will be used
        //     to destroy any thread-specific object when "reset()" is called, or thread
        //     exits. If cleanup == NULL, the thread-specific object will not be destroyed.
        // Throws: wsd::SyscallException if thread-specific storage could not be allocated.
        ThreadSpecificPtr(void (*cleanup)(void*))
            : m_key(0),
              m_cleanup(cleanup)
        {
            int err = pthread_key_create(&m_key, m_cleanup);
            if (err)
                throw SyscallException(__FILE__, __LINE__, err);
        }

        // Return the pointer associated with current thread.
        //
        // Throws: nothing.
        T *get() const
        {
            return static_cast<T*>(pthread_getspecific(m_key));
        }
        
        // Return "this->get()".
        // 
        // Throws: nothing
        T *operator->() const
        {
            return get();
        }

        // Return "*(this->get())".
        //
        // Requires: this->get() != NULL.
        // Throws: nothing
        T& operator*() const
        {
            return *get();
        }

        operator unspecified_bool_type() const
        {
            return get() ? &ThreadSpecificPtr::get : NULL;
        }
        
        // Effects: If "this->get() != new_value", invoke "delete this->get()" or
        //     "cleanup(this->get())" as appropriate. Store "new_value" as the pointer
        //     associated with current thread.
        // Throws: nothing
        void reset(T *new_value = NULL)
        {
            T *current_value = get();
            if (current_value != new_value) {
                int err = pthread_setspecific(m_key, new_value);
                WSD_ASSERT(err == 0);
                if (m_cleanup)
                    m_cleanup(current_value);
            }
        }

        // Effects: Return "this->get()" and store NULL as the pointer associated with current
        //     thread without calling cleanup function.
        // Throws: nothing
        T *release()
        {
            T *current_value = get();
            int err = pthread_setspecific(m_key, NULL);
            WSD_ASSERT(err == 0);
            return current_value;
        }
            
    private:

        DISALLOW_COPY_AND_ASSIGN(ThreadSpecificPtr);
        
        static void delete_data(void *p)
        {
            checked_delete(static_cast<T*>(p));
        }
        
        mutable pthread_key_t m_key;  // key of the contained thread-specific storage

        // Cleanup function for the thread-specific storage. NULL value means an no-op cleanup.
        void (*m_cleanup)(void*);
    };

}  // namespace wsd

#endif  // __TSS_H__
