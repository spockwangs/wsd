#ifndef __SHARED_PTR_H__
#define __SHARED_PTR_H__

#include "wsd_assert.h"
#include <iostream>
#include "detail/shared_count_impl.h"
#include "checked_delete.h"
#include "template_util.h"

namespace wsd {
    
    template <typename T> class SharedPtr;
    template <typename T> class EnableSharedFromThis;

    namespace detail {

        // EnableSharedFromThis support

        template <class X, class T>
        inline void sp_enable_shared_from_this(const wsd::SharedPtr<X> *pp,
                                               const wsd::EnableSharedFromThis<T> * px)
        {
            if (px != NULL) {
                px->acceptOwner(*pp);
            }
        }

        inline void sp_enable_shared_from_this(...)
        {
        }

    } // namespace detail
    
    template <typename T>
    class SharedPtr {
    public:
        typedef T* (SharedPtr::*unspecified_bool_type)() const;
        
        /**
         * \throws nothing
         */
        SharedPtr()
            : m_px(NULL),
              m_pn(NULL)
        { }
        
        /**
         * \pre p must be convertible to T*; U must be complete; "delete p" must
         *      not throw exception.
         * \post this->usecount() becomse 1 on success. If an exception is thrown "delete
         *      p" is called to deallocate the memory.
         * \throws may throw std::bad_alloc or other implementation defined exception
         */
        template <typename U>
        explicit SharedPtr(U *p)
            : m_px(p),
              m_pn(NULL)
        {
            try {
                if (m_px)
                    m_pn = new detail::shared_count_impl_p<U>(p);
            } catch (...) {
                checked_delete(p);
                throw;
            }
            detail::sp_enable_shared_from_this(this, p);
        }

        // Requirements: p must be convertible to T; U must be a complete type; D must be
        //     CopyConstructible; copy constructor and destructor of D must not throw;
        //     d(p) must not throw exceptions.
        // Throws: may throw std::bad_alloc, or other implementation-defined exceptions
        template <typename U, typename D>
        SharedPtr(U *p, D d)
            : m_px(p),
              m_pn(NULL)
        {
            try {
                if (m_px)
                    m_pn = new detail::shared_count_impl_pd<U, D>(p, d);
            } catch (...) {
                d(p);
                throw;
            }
            detail::sp_enable_shared_from_this(this, p);
        }

        /**
         * \throws nothing
         */
        SharedPtr(const SharedPtr& rhs)
            : m_px(rhs.m_px),
              m_pn(rhs.m_pn)
        {
            if (m_pn)
                m_pn->increment();
        }

        /**
         * \pre U* must be convertible to T*
         * \throws nothing
         */
        template <typename U>
        SharedPtr(const SharedPtr<U>& rhs, typename enable_if<is_convertible<U*, T*> >::type* = 0)
            : m_px(rhs.m_px),
              m_pn(rhs.m_pn)
        {
            if (m_pn)
                m_pn->increment();
        }
    
        ~SharedPtr()
        {
            WSD_ASSERT((m_px && m_pn && m_pn->use_count() > 0)
                       || (m_px == NULL && m_pn == NULL));
            
            if (m_pn)
                m_pn->release();
        }

        /**
         * \throws nothing
         */
        SharedPtr& operator=(const SharedPtr& rhs)
        {
            SharedPtr(rhs).swap(*this);
            return *this;
        }
        
        /**
         * \throws nothing
         */
        template <typename U>
        SharedPtr& operator=(const SharedPtr<U>& rhs)
        {
            SharedPtr(rhs).swap(*this);
            return *this;
        }
    
        /**
         * \throws nothing
         */
        T *operator->() const
        {
            WSD_ASSERT(m_px);
            return m_px;
        }
        
        T& operator*() const
        {
            WSD_ASSERT(m_px);
            return *m_px;
        }
        
        T *get() const
        { return m_px; }

        operator unspecified_bool_type() const
        {
            return m_px == NULL ? NULL : &SharedPtr::get;
        }

        size_t usecount() const
        {
            if (m_pn)
                return m_pn->use_count();
            return 1;
        }
    
        bool unique() const
        {
            return usecount() == 1;
        }
        
        void reset(T *p = NULL)
        {
            SharedPtr(p).swap(*this);
        }

        /**
         * \throws std::bad_alloc if memory is unavailable.
         */
        template <typename D>
        void reset(T *p, D d)
        {
            SharedPtr(p, d).swap(*this);
        }
        
        void swap(SharedPtr& other)
        {
            std::swap(m_px, other.m_px);
            std::swap(m_pn, other.m_pn);
        }

    private:

        // EnableSharedFromThis support
        template <typename U>
        explicit SharedPtr(U *p, detail::shared_count_base<> *pn)
            : m_px(p), m_pn(pn)
        {
            if (m_pn == NULL)
                SharedPtr(p);
            else
                m_pn->increment();
        }

        T *m_px;
        detail::shared_count_base<> *m_pn;
        
        template <typename U> friend class SharedPtr;
        template <typename U> friend class EnableSharedFromThis;
    };

    template <typename T>
    inline bool operator==(const SharedPtr<T>& lhs, const SharedPtr<T>& rhs)
    {
        return lhs.get() == rhs.get();
    }

    template <typename T>
    inline bool operator!=(const SharedPtr<T>& lhs, const SharedPtr<T>& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename T>
    inline bool operator<(const SharedPtr<T>& lhs, const SharedPtr<T>& rhs)
    {
        return lhs.get() < rhs.get();
    }

    template <typename T>
    inline bool operator>(const SharedPtr<T>& lhs, const SharedPtr<T>& rhs)
    {
        return rhs.get() < lhs.get();
    }

    template <typename T>
    inline std::ostream& operator<<(std::ostream& os, const SharedPtr<T>& p)
    {
        os << p.get();
        return os;
    }

    template <typename T>
    inline void swap(SharedPtr<T>& lhs, SharedPtr<T>& rhs)
    {
        lhs.swap(rhs);
    }
    
} // namespace wsd

#endif
