// Copyright (c) 2013 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//
// @file singleton.h
//
// The Singleton template is used to control the creation of an object, not to
// implement the singleton pattern.  The singleton pattern is discouraged because it is
// just a delicate global variable.  So the singleton template is designed to prevent from
// being inherited.
//
// Given some type T, you can create an object of type T like this:
//
//     const wsd::SharedPtr<T>& ptr = wsd::Singleton<T>::getInstance();
//
// The object is only created once the first time you access it, and the same object will
// be returned when being accessed later.  The method `getInstance()' is thread-safe.

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

    private:
        DISALLOW_COPY_AND_ASSIGN(Singleton);
        
        // Made private to prevent from being inherited.
        Singleton() {}
        
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
