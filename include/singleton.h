// Copyright (c) 2013 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include "tss.h"
#include "shared_ptr.h"
#include "mutex.h"

namespace wsd {

    namespace detail {

        struct no_deleter {
            void operator()(void *) {}
        };

    }  // namespace detail
            
    template <typename T>
    class Singleton {
    public:
        static SharedPtr<T> getInstance()
        {
            if (!s_instance) {
                {
                    Mutex::Lock lock(s_mutex);
                    if (!s_p)
                        s_p = new T();
                }
                s_instance.reset(s_p);
            }
            return SharedPtr<T>(s_instance.get(), detail::no_deleter());
        }

    protected:
        Singleton() {}
        
    private:
        DISALLOW_COPY_AND_ASSIGN(Singleton);
        
        static Mutex s_mutex;
        static T *s_p;
        static ThreadSpecificPtr<T> s_instance;
    };

    template <typename T>
    Mutex Singleton<T>::s_mutex;

    template <typename T>
    T *Singleton<T>::s_p = NULL;

    template <typename T>
    ThreadSpecificPtr<T> Singleton<T>::s_instance;

}  // namespace wsd

#endif  // __SINGLETON_H__
