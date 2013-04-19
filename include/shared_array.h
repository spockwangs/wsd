#ifndef __SHARED_ARRAY_H__
#define __SHARED_ARRAY_H__

#include "wsd_assert.h"
#include <iostream>
#include "detail/shared_count_impl.h"
#include "checked_delete.h"

namespace wsd {

    template <typename T>
    class SharedArray {
    public:

        typedef T* (SharedArray::*unspecified_bool_type)() const;
        
        /**
         * Constructs a SharedArray object, which stores a copy of `p'. If an exception is
         * thrown, `delete[] p' is called.
         *
         * \pre p must be a pointer to an array that was allocated via a C++ new[]
         *     expression or be NULL. `delete[] p' must not throw exceptions.
         * \post this->get() == p && this->usecount() == 1 even if p == NULL
         * \throws std::bad_alloc
         */
        explicit SharedArray(T *p = NULL)
            : m_px(p),
              m_pn(NULL)
        {
            try {
                if (m_px) {
                    void (*deleter)(T*) = &checked_array_delete<T>;
                    m_pn = new detail::shared_count_impl_pd<T, void (*)(T*)>(p, deleter);
                }
            } catch (...) {
                checked_array_delete(p);
                throw;
            }
        }

        /**
         * Constructs a SharedArray object, which stores a copy of `p'.  When the time
         * comes to delete the array pointed to by `p', it is deleted by calling
         * `d(p)'. If an exception is thrown `d(p)' is called.
         *
         * \pre D's copy constructor and destructor must not throw. `d(p)' must not throw.
         * \throws std::bad_alloc
         */
        template <typename D>
        SharedArray(T *p, D d)
            : m_px(p),
              m_pn(NULL)
        {
            try {
                if (m_px)
                    m_pn = new detail::shared_count_impl_pd<T, D>(p, d);
            } catch (...) {
                d(p);
                throw;
            }
        }
        
        /**
         * Constructs a SharedArray object by stroing a copy of the pointer stored in
         * `rhs'. Afterwards the use count increases by 1.
         *
         * \throws nothing
         */
        SharedArray(const SharedArray& rhs)
            : m_px(rhs.m_px),
              m_pn(rhs.m_pn)
        {
            if (m_pn)
                m_pn->increment();
        }

        /**
         * Decrements the use count. If the use count is 0, delete the array pointed to by
         * the stored pointer.
         * 
         * \throws nothing
         */
        ~SharedArray()
        {
            WSD_ASSERT((m_px && m_pn && m_pn->use_count() > 0)
                       || (m_px == NULL && m_pn == NULL));
            if (m_pn)
                m_pn->release();
        }

        SharedArray& operator=(const SharedArray& rhs)
        {
            SharedArray(rhs).swap(*this);
            return *this;
        }
        
        /**
         * Returns a reference to element i of the array pointed to by the stored pointer.
         *
         * \pre this->get() != NULL && i >= 0 && i <= number of elements in the array
         * \throws nothing
         */
        T& operator[](std::ptrdiff_t i) const
        {
            WSD_ASSERT(m_px != NULL && i >= 0);
            return m_px[i];
        }
        
        /**
         * Returns the stored pointer.
         *
         * \throws nothing.
         */
        T *get() const
        { return m_px; }

        operator unspecified_bool_type() const
        { return m_px ? &SharedArray::get : 0; }
            
        /**
         * Returns the number of SharedArray objects that shares the ownership of the
         * stored pointer.
         *
         * \throws nothing
         */
        size_t usecount() const
        {
            if (m_pn)
                return m_pn->use_count();
            return 1;
        }
    
        /**
         * Returns true if only one SharedArray object shares the onwership of the stored pointer.
         * 
         * \throws nothing
         */
        bool unique() const
        { return usecount() == 1; }
        
        /**
         * Constructs a new SharedArray object by calling `SharedArray(p)' and replace this one.
         * 
         * \throws std::bad_alloc
         */
        void reset(T *p = NULL)
        {
            SharedArray(p).swap(*this);
        }

        /**
         * Constructs a new SharedArray object by calling `SharedArray(p, d)' and replace
         * this one.
         *
         * \throws std::bad_alloc
         */
        template <typename D>
        void reset(T *p, D d)
        {
            SharedArray(p, d).swap(*this);
        }
        
        /**
         * Swap the stored pointer and use count with another SharedArray object.
         *
         * \throws nothing
         */
        void swap(SharedArray& other)
        {
            std::swap(m_px, other.m_px);
            std::swap(m_pn, other.m_pn);
        }

    private:
        
        T *m_px;
        detail::shared_count_base<> *m_pn;
    };

    template <typename T>
    inline bool operator==(const SharedArray<T>& lhs,
                           const SharedArray<T>& rhs)
    {
        return lhs.get() == rhs.get();
    }

    template <typename T>
    inline bool operator!=(const SharedArray<T>& lhs,
                           const SharedArray<T>& rhs)
    {
        return lhs.get() != rhs.get();
    }

    template <typename T>
    inline bool operator<(const SharedArray<T>& lhs,
                          const SharedArray<T>& rhs)
    {
        return std::less<T*>()(lhs.get(), rhs.get());
    }

    template <typename T>
    inline void swap(SharedArray<T>& lhs, SharedArray<T>& rhs)
    {
        lhs.swap(rhs);
    }

}  // namespace wsd

#endif  // __SHARED_ARRAY_H__
