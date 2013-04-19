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

    template <typename T>
    class Singleton {
    public:
        static SharedPtr<T> getInstance()
        {
            if (!s_instance) {
                {
                    Mutex::Lock lock(s_mutex);
                    if (!s_ptr)
                        s_ptr.reset(new T);
                }
                s_instance.reset(&s_ptr);
            }
            return *s_instance;
        }

    private:
        DISALLOW_COPY_AND_ASSIGN(Singleton);
        
        // Made private to prevent from being inherited.
        Singleton() {}
        
        static Mutex s_mutex;
        static SharedPtr<T> s_ptr;
        static ThreadSpecificPtr<SharedPtr<T> > s_instance;
    };

    template <typename T>
    Mutex Singleton<T>::s_mutex;

    template <typename T>
    SharedPtr<T> Singleton<T>::s_ptr;

    // Use thread-local storage to cache the pointer to the singleton object to decrease
    // contention.  Note that don't delete the thread-local data on thread exit because
    // the data is statically allocated.
    template <typename T>
    ThreadSpecificPtr<SharedPtr<T> > Singleton<T>::s_instance(NULL);  

}  // namespace wsd

#endif  // __SINGLETON_H__
