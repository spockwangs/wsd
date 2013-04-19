// -*- coding: utf-8-unix; -*-
#ifndef __ENABLE_SHARED_FROM_THIS_H__
#define __ENABLE_SHARED_FROM_THIS_H__

#include "shared_ptr.h"

namespace wsd {

    template <class T>
    class EnableSharedFromThis {
    public:

        /**
         * \pre `EnableSharedFromThis<T>' must be an accessible base class of `T'. `*this'
         *     must be a subobject of an instance `t' of type `T'.
         * \returns A `SharedPtr<T>' instance that shares onwership with `p' if there is a
         *     `SharedPtr' instance `p' that owns `t', or an NULL `SharedPtr<T>' instance.
         */
        SharedPtr<T> sharedFromThis()
        {
            SharedPtr<T> p(m_this, m_owner_use_count);
            return p;
        }

        /**
         * Same as above.
         */
        SharedPtr<const T> sharedFromThis() const
        {
            SharedPtr<const T> p(m_this, m_owner_use_count);
            return p;
        }
        
    protected:
        EnableSharedFromThis()
            : m_this(NULL),
              m_owner_use_count(NULL)
        { }

        EnableSharedFromThis(const EnableSharedFromThis&)
        { }

        EnableSharedFromThis& operator=(const EnableSharedFromThis&)
        {
            return *this;
        }

        // User can not delete a pointer of type EnableSharedFromThis* so it is not
        // necessary to be virtual.
        ~EnableSharedFromThis()
        { }
        
    private:
        
        template <typename U>
        void acceptOwner(const SharedPtr<U>& p) const
        {
            if (m_owner_use_count == NULL) {
                m_owner_use_count = p.m_pn;
                m_this = p.get();
            }
        }

        mutable T *m_this;
        mutable detail::shared_count_base<> *m_owner_use_count;

        template <class X, class U>
        friend void detail::sp_enable_shared_from_this(const SharedPtr<X> *pp,
                                                       const EnableSharedFromThis<U> *px);
    };
    
} // namespace wsd

#endif
