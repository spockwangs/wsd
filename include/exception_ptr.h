// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __EXCEPTION_PTR_H__
#define __EXCEPTION_PTR_H__

#include "shared_ptr.h"
#include "wsd_exception.h"
#include <stdexcept>
#include <new>
#include <typeinfo>

namespace wsd {

    namespace detail {

        class ExceptionBase {
        public:
            virtual ~ExceptionBase() {}

            virtual ExceptionBase* clone() const = 0;

            virtual void rethrow() const = 0;
        };

        template <typename E>
        class ExceptionBaseImpl : public ExceptionBase, public E {
        public:
            ExceptionBaseImpl() {}
            
            ExceptionBaseImpl(const E& e)
                : E(e)
            {}

            virtual ~ExceptionBaseImpl() throw() {}

            virtual ExceptionBase* clone() const
            {
                return new ExceptionBaseImpl(*this);
            }

            virtual void rethrow() const
            {
                throw *this;
            }
        };

    }  // namespace detail
    
    typedef SharedPtr<detail::ExceptionBase> ExceptionPtr;

    namespace detail {
        
        template <typename E>
        struct ExceptionPtrStaticExceptionObject {
            static const ExceptionPtr e;
        };

        template <typename E>
        const ExceptionPtr ExceptionPtrStaticExceptionObject<E>::e(new ExceptionBaseImpl<E>());

    }  // namespace detail

    class CurrentExceptionUnknownException : WsdException {
    private:

        const char *name() const
        { return "CurrentExceptionUnknownException"; }
    };

    /**
     * \pre This function must be called in a catch block.
     * \throws nothing
     */
    inline ExceptionPtr currentException()
    {
        try {
            try {
                throw;
            } catch (detail::ExceptionBase& e) {
                return ExceptionPtr(e.clone());
            } catch (std::domain_error& e) {
                return ExceptionPtr(new detail::ExceptionBaseImpl<std::domain_error>(e));
            } catch (std::invalid_argument& e) {
                return ExceptionPtr(new detail::ExceptionBaseImpl<std::invalid_argument>(e));
            } catch (std::length_error& e) {
                return ExceptionPtr(new detail::ExceptionBaseImpl<std::length_error>(e));
            } catch (std::out_of_range& e) {
                return ExceptionPtr(new detail::ExceptionBaseImpl<std::out_of_range>(e));
            } catch (std::range_error& e) {
                return ExceptionPtr(new detail::ExceptionBaseImpl<std::range_error>(e));                
            } catch (std::overflow_error& e) {
                return ExceptionPtr(new detail::ExceptionBaseImpl<std::overflow_error>(e));
            } catch (std::underflow_error& e) {
                return ExceptionPtr(new detail::ExceptionBaseImpl<std::underflow_error>(e));
            } catch (std::logic_error& e) {
                return ExceptionPtr(new detail::ExceptionBaseImpl<std::logic_error>(e));
            } catch (std::bad_alloc& e) {
                return ExceptionPtr(new detail::ExceptionBaseImpl<std::bad_alloc>(e));
            } catch (std::bad_cast& e) {
                return ExceptionPtr(new detail::ExceptionBaseImpl<std::bad_cast>(e));
            } catch (std::bad_typeid& e) {
                return ExceptionPtr(new detail::ExceptionBaseImpl<std::bad_typeid>(e));
            } catch (std::bad_exception& e) {
                return ExceptionPtr(new detail::ExceptionBaseImpl<std::bad_exception>(e));
            } catch (WsdException& e) {
                return ExceptionPtr(new detail::ExceptionBaseImpl<WsdException>(e));
            } catch (std::exception& e) {
                return ExceptionPtr(new detail::ExceptionBaseImpl<std::exception>(e));
            }
        } catch (std::bad_alloc& e) {
            return detail::ExceptionPtrStaticExceptionObject<std::bad_alloc>::e;
        } catch (...) {
            return detail::ExceptionPtrStaticExceptionObject<CurrentExceptionUnknownException>::e;
        }
    }

    /**
     * This function is used in a throw-expression to enable the ExceptionPtr support.
     * 
     * \pre T must be a class with public no-throw copy constructor.
     * \returns an object of unspecified type which derives publicly from T
     */
    template <typename T>
    inline detail::ExceptionBaseImpl<T> enableCurrentException(const T& t)
    {
        return detail::ExceptionBaseImpl<T>(t);
    }

    template <typename E>
    inline void throwException(const E& e)
    {
        throw enableCurrentException(e);
    }

    /**
     * \pre E must be a class type with public no-throw copy constructor.
     * \returns an object of ExceptionPtr which refers to a copy of `e'.
     * \throws nothing
     */
    template <typename E>
    inline ExceptionPtr copyException(const E& e)
    {
        try {
            throw enableCurrentException(e);
        } catch (...) {
            return currentException();
        }
    }
    
}  // namespace wsd

#endif  // __EXCEPTION_PTR_H__
