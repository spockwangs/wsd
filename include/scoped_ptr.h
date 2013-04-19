#ifndef __SCOPED_PTR_H__
#define __SCOPED_PTR_H__

#include "wsd_assert.h"
#include "wsd_magic.h"
#include "checked_delete.h"

namespace wsd {

    template <typename T>
    class ScopedPtr {
    public:
        typedef T element_type;
        typedef T* (ScopedPtr::*unspecified_bool_type)() const;
        
        explicit ScopedPtr(T *p = NULL)
            : m_p(p)
        { }
            
        // Never throw. The destructor of pointee is required to never throw.
        ~ScopedPtr()
        {
            checked_delete(m_p);
        }

        /**
         * Return the object this pointer is pointing to.
         *
         * \pre `this->get() != NULL`
         */
        T& operator*() const
        {
            WSD_ASSERT(m_p != NULL);
            return *m_p;
        }
                
        /**
         * Returns `this->get()`.
         *
         * \pre `this->get() != NULL`.
         */
        T *operator->() const
        {
            WSD_ASSERT(m_p != NULL);
            return m_p;
        }
        
        /**
         * Returns the stored pointer.
         */
        T *get() const
        {
            return m_p;
        }

        /**
         * Returns an value that, when used in boolean contexts, is equivalent to
         * `get() != NULL'.
         */
        operator unspecified_bool_type() const
        {
            return m_p ? &ScopedPtr::get : 0;
        }

        void reset(T *p = NULL)
        {
            ScopedPtr(p).swap(*this);
        }

        void swap(ScopedPtr& other)
        {
            std::swap(m_p, other.m_p);
        }

    private:
        DISALLOW_COPY_AND_ASSIGN(ScopedPtr);
        
        T *m_p;
    };

} // namespace wsd

#endif
