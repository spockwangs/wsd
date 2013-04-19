// -*- coding: utf-8-unix; -*-
// Copyright (c) 2012 Tencent, Inc.
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#ifndef __SCOPED_ARRAY_H__
#define __SCOPED_ARRAY_H__

#include "wsd_assert.h"
#include "wsd_magic.h"
#include "checked_delete.h"

namespace wsd {

    /**
     * The ScopedArray class template stores a pointer to dynamically allocated array. The
     * array pointer is guaranteed to be deleted, either on destruction of the ScopedArray,
     * or via an explicit `reset()'.
     */
    template <typename T>
    class ScopedArray {
    public:
        typedef T element_type;
        typedef T* (ScopedArray::*unspecified_bool_type)() const;
        
        explicit ScopedArray(T *p = NULL)
            : m_p(p)
        { }
            
        // Never throw. The destructor of pointee is required to never throw.
        ~ScopedArray()
        {
            checked_array_delete(m_p);
        }

        /**
         * Returns the stored pointer.
         *
         * \throws nothing
         */
        T *get() const
        {
            return m_p;
        }

        /**
         * Returns a reference to element i of the array pointed to by the stored pointer.
         *
         * \pre this->get() != NULL && i >= 0 && i < number of elements in the array
         * \throws nothing
         */
        T& operator[](std::ptrdiff_t i) const
        {
            WSD_ASSERT(m_p != NULL && i >= 0);
            return m_p[i];
        }
        
        /**
         * Returns an value that, when used in boolean contexts, is equivalent to
         * `get() != NULL'.
         */
        operator unspecified_bool_type() const
        {
            return m_p ? &ScopedArray::get : 0;
        }

        /**
         * Deletes the array pointed to by the stored pointer and stores a copy of p.
         *
         * \pre p must point to dynamically allocated array, or be NULL
         * \throws nothing if the deleted array's object's destructors do not throw exceptions.
         */
        void reset(T *p = NULL)
        {
            ScopedArray(p).swap(*this);
        }

        /**
         * Swap the stored pointer of two ScopedArray objects.
         *
         * \throws nothing
         */
        void swap(ScopedArray& other)
        {
            std::swap(m_p, other.m_p);
        }

    private:
        DISALLOW_COPY_AND_ASSIGN(ScopedArray);
        
        T *m_p;
    };

} // namespace wsd

#endif  // __SCOPED_ARRAY_H__
