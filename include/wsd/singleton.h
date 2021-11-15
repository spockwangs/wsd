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

#include "boost/shared_ptr.hpp"
#include "boost/thread/locks.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/tss.hpp"
#include "wsd_magic.h"

namespace wsd {

template <typename T>
class Singleton {
public:
    static boost::shared_ptr<T> getInstance()
    {
        if (s_instance.get() == NULL) {
            {
                boost::lock_guard<boost::mutex> lock(s_mutex);
                if (!s_ptr) s_ptr.reset(new T);
            }
            s_instance.reset(&s_ptr);
        }
        return *s_instance;
    }

private:
    DISALLOW_COPY_AND_ASSIGN(Singleton);

    // Made private to prevent from being inherited.
    Singleton()
    {
    }

    static boost::mutex s_mutex;
    static boost::shared_ptr<T> s_ptr;
    static boost::thread_specific_ptr<boost::shared_ptr<T>> s_instance;
};

template <typename T>
boost::mutex Singleton<T>::s_mutex;

template <typename T>
boost::shared_ptr<T> Singleton<T>::s_ptr;

// Use thread-local storage to cache the pointer to the singleton object to decrease
// contention.  Note that don't delete the thread-local data on thread exit because
// the data is statically allocated.
template <typename T>
boost::thread_specific_ptr<boost::shared_ptr<T>> Singleton<T>::s_instance(NULL);

}  // namespace wsd

#endif  // __SINGLETON_H__
