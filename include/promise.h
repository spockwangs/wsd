// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __PROMISE_H__
#define __PROMISE_H__

#include "callback.h"
#include "exception_ptr.h"
#include <pthread.h>
#include <list>
#include "bind.h"
#include "enable_shared_from_this.h"
#include "lock.h"
#include "cond.h"
#include "wsd_assert.h"
#include "wsd_magic.h"
#include "template_util.h"

namespace wsd {

    class PromiseAlreadySatisfiedException : public WsdException {
    public:

        PromiseAlreadySatisfiedException(const char *filename, int line)
            : WsdException(filename, line)
        {}
        
    protected:

        virtual const char *name() const
        {
            return "PromiseAlreadySatisfiedException";
        }
    };

    class FutureUninitialized : public WsdException {
    public:
        FutureUninitialized(const char *filename, int line)
            : WsdException(filename, line)
        {}

    protected:
        virtual const char *name() const
        {
            return "FutureUninitialized";
        }
    };
    
    template <typename T> class Future;

    template <typename T> struct is_future_type : false_type {};
    template <typename T> struct is_future_type<Future<T> > : true_type {};

    template <typename T> class Promise;
    
    namespace detail {

        template <typename T>
        struct resolved_type {
            typedef T type;
        };

        template <typename T>
        struct resolved_type<Future<T> > {
            typedef T type;
        };
    
        template <typename T>
        struct FutureTraits {
            typedef ScopedPtr<T> storage_type;
            typedef const T& rvalue_source_type;
            typedef const T& move_dest_type;
            typedef T& dest_reference_type;

            static void init(storage_type& storage, rvalue_source_type t)
            {
                storage.reset(new T(t));
            }

            static void assign(dest_reference_type dest, const storage_type& storage)
            {
                dest = *storage;
            }
        };

        template <typename T>
        struct FutureTraits<T&> {
            typedef T* storage_type;
            typedef T& rvalue_source_type;
            typedef T& move_dest_type;
            typedef T*& dest_reference_type;

            static void init(storage_type& storage, T& t)
            {
                storage = &t;
            }

            static void assign(dest_reference_type dest, const storage_type& storage)
            {
                dest = storage;
            }
        };
            
        template <>
        struct FutureTraits<void> {
            typedef void rvalue_source_type;
            typedef void move_dest_type;
        };
        
        template <typename T>
        class FutureObjectInterface {
        public:
            typedef typename FutureTraits<T>::rvalue_source_type rvalue_source_type;
            typedef typename FutureTraits<T>::move_dest_type move_dest_type;
            typedef typename FutureTraits<T>::dest_reference_type dest_reference_type;
            typedef Callback<void(const SharedPtr<FutureObjectInterface>&)> CallbackType;
            
            virtual ~FutureObjectInterface() {}

            virtual bool isDone() const = 0;
            virtual bool hasValue() const = 0;
            virtual bool hasException() const = 0;
            virtual void setValue(rvalue_source_type v) = 0;
            virtual void setException(const ExceptionPtr& e) = 0;
            virtual move_dest_type get() const = 0;
            virtual bool tryGet(dest_reference_type v) const = 0;
            virtual void registerCallback(const CallbackType& callback) = 0;
        };
            
        template <>
        class FutureObjectInterface<void> {
        public:
            typedef FutureTraits<void>::move_dest_type move_dest_type;
            typedef Callback<void(const SharedPtr<FutureObjectInterface>&)> CallbackType;
            
            virtual ~FutureObjectInterface() {}

            virtual bool isDone() const = 0;
            virtual bool hasValue() const = 0;
            virtual bool hasException() const = 0;
            virtual void set() = 0;
            virtual void setException(const ExceptionPtr& e) = 0;
            virtual move_dest_type get() const = 0;
            virtual void registerCallback(const CallbackType& callback) = 0;
        };
        
        // Optimized implementation of FutureObjectInterface<> for prompt futures, which
        // does not require locking.
        //
        // See section 2.6 of [http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3327.pdf].
        template <typename T>
        class PromptFutureObject : public FutureObjectInterface<T>,
                                   public EnableSharedFromThis<PromptFutureObject<T> > {
        public:
            typedef typename FutureObjectInterface<T>::move_dest_type move_dest_type;
            typedef typename FutureObjectInterface<T>::rvalue_source_type rvalue_source_type;
            typedef typename FutureObjectInterface<T>::dest_reference_type dest_reference_type;
            typedef typename FutureObjectInterface<T>::CallbackType CallbackType;
            
            PromptFutureObject(rvalue_source_type v)
            {
                FutureTraits<T>::init(m_value, v);
            }

            PromptFutureObject(const ExceptionPtr& e)
                : m_exception_ptr(e)
            {}
            
            virtual ~PromptFutureObject() {}

            virtual bool isDone() const { return true; }

            virtual bool hasValue() const
            {
                if (m_exception_ptr)
                    return false;
                return true;
            }
            
            virtual bool hasException() const { return m_exception_ptr; }

            // Promise<> will never use this implementation.
            virtual void setValue(rvalue_source_type /*v*/)
            {
                WSD_FAIL();
            }

            virtual void setException(const ExceptionPtr& /*e*/)
            {
                WSD_FAIL();
            }
            
            virtual move_dest_type get() const
            {
                WSD_ASSERT(m_exception_ptr || m_value);
                if (m_exception_ptr)
                    m_exception_ptr->rethrow();
                return *m_value;
            }
            
            virtual bool tryGet(dest_reference_type v) const { FutureTraits<T>::assign(v, m_value); return true; }
            
            virtual void registerCallback(const CallbackType& callback)
            {
                WSD_ASSERT(callback);
                try {
                    callback(this->sharedFromThis());
                } catch (...) {
                    // Ingore the exception thrown by the callback.
                }
            }
            
        private:
            typename FutureTraits<T>::storage_type m_value;
            ExceptionPtr m_exception_ptr;
        };
        
        template <>
        class PromptFutureObject<void> : public FutureObjectInterface<void>,
                                         public EnableSharedFromThis<PromptFutureObject<void> > {
        public:
            typedef FutureObjectInterface<void>::move_dest_type move_dest_type;
            typedef FutureObjectInterface<void>::CallbackType CallbackType;
            
            PromptFutureObject() {}

            PromptFutureObject(const ExceptionPtr& e)
                : m_exception_ptr(e)
            {}
            
            virtual ~PromptFutureObject() {}

            virtual bool isDone() const { return true; }

            virtual bool hasValue() const
            {
                if (m_exception_ptr)
                    return false;
                return true;
            }
            
            virtual bool hasException() const { return m_exception_ptr; }

            // Promise<> will not never this implementation.
            virtual void set()
            {
                WSD_FAIL();
            }
            
            virtual void setException(const ExceptionPtr& /*e*/)
            {
                WSD_FAIL();
            }
            
            virtual move_dest_type get() const
            {
                if (m_exception_ptr)
                    m_exception_ptr->rethrow();
            }
            
            virtual void registerCallback(const CallbackType& callback)
            {
                WSD_ASSERT(callback);
                try {
                    callback(this->sharedFromThis());
                } catch (...) {
                    // Ingore the exception thrown by the callback.
                }
            }
            
        private:
            ExceptionPtr m_exception_ptr;
        };

        struct FutureObjectBase {
            FutureObjectBase()
                : m_is_done(false)
            {}

            virtual ~FutureObjectBase() {}
            
            bool isDone() const
            {
                Mutex::Lock lock(m_mutex);
                return m_is_done;
            }
            
            bool hasValue() const
            {
                Mutex::Lock lock(m_mutex);
                return m_is_done && !m_exception_ptr;
            }

            bool hasException() const
            {
                Mutex::Lock lock(m_mutex);
                return m_is_done && m_exception_ptr;
            }
            
            void markFinishedWithException(const ExceptionPtr& e)
            {
                WSD_ASSERT(e);
                Mutex::Lock lock(m_mutex);
                if (m_is_done) {
                    throw PromiseAlreadySatisfiedException(__FILE__, __LINE__);
                }
                m_exception_ptr = e;
                markFinishedInternal();
            }

            void markFinishedInternal()
            {
                m_is_done = true;
                m_cond.broadcast();
            }

            void wait() const
            {
                {
                    Mutex::Lock lock(m_mutex);
                    while (!m_is_done) {
                        // This promise is not satisfied yet. So we wait.
                        m_cond.wait(m_mutex);
                    }
                }

                if (m_exception_ptr)
                    m_exception_ptr->rethrow();
            }

            Mutex m_mutex;
            Cond m_cond;  // predicate: m_is_done == true
            bool m_is_done;  // either a value or exception is set
            ExceptionPtr m_exception_ptr;
        };
        
        template <typename T>
        class FutureObject : public FutureObjectInterface<T>,
                             public EnableSharedFromThis<FutureObject<T> >,
                             private FutureObjectBase {
        public:

            typedef typename FutureObjectInterface<T>::move_dest_type move_dest_type;
            typedef typename FutureObjectInterface<T>::rvalue_source_type rvalue_source_type;
            typedef typename FutureObjectInterface<T>::CallbackType CallbackType;
            typedef typename FutureObjectInterface<T>::dest_reference_type dest_reference_type;
            
            FutureObject() {}

            virtual ~FutureObject() {}
            
            virtual bool isDone() const
            {
                return FutureObjectBase::isDone();
            }
            
            virtual bool hasValue() const
            {
                return FutureObjectBase::hasValue();
            }

            virtual bool hasException() const
            {
                return FutureObjectBase::hasException();
            }
            
            virtual move_dest_type get() const
            {
                wait();

                WSD_ASSERT(m_value);
                return *m_value;
            }

            virtual bool tryGet(dest_reference_type v) const
            {
                if (!isDone())
                    return false;
                if (m_exception_ptr)
                    m_exception_ptr->rethrow();
                FutureTraits<T>::assign(v, m_value);
                return true;
            }
            
            virtual void setException(const ExceptionPtr& e)
            {
                markFinishedWithException(e);
                doPendingCallbacks();
            }
            
            virtual void setValue(rvalue_source_type t)
            {
                {
                    Mutex::Lock lock(m_mutex);
                    if (m_is_done) {
                        throwException(PromiseAlreadySatisfiedException(__FILE__, __LINE__));
                    }
                    FutureTraits<T>::init(m_value, t);
                    markFinishedInternal();
                }
                doPendingCallbacks();
            }
            
            // pre: `callback' should not throw an exception because it may be run in
            // another thread at a later time so the exception can not be caught by the
            // caller and is meaningless. And this object must be shared by some
            // SharedPtr<FutureObject<T> > instance.
            virtual void registerCallback(const CallbackType& callback)
            {
                WSD_ASSERT(callback);
                Mutex::Lock lock(m_mutex);
                if (m_is_done) {
                    lock.release();
                    try {
                        callback(this->sharedFromThis());
                    } catch (...) {
                        // Ignore the exceptions thrown by the callback.
                    }
                } else {
                    m_pending_callbacks.push_back(callback);
                }
            }

        private:

            // Note: this function must be called without locks, otherwise deadlock will
            // happen because the callback passed by users may want to acquire the lock.
            void doPendingCallbacks()
            {
                std::list<CallbackType> callbacks;
                {
                    Mutex::Lock lock(m_mutex);
                    callbacks.swap(m_pending_callbacks);
                }

                for (typename std::list<CallbackType>::const_iterator it = callbacks.begin(),
                                                                     end = callbacks.end();
                        it != end; ++it) {
                    try {
                        (*it)(this->sharedFromThis());
                    } catch (...) {
                        // Ignore the exceptions thrown by the callback.
                    }
                }
            }

            typename FutureTraits<T>::storage_type m_value;
            std::list<CallbackType> m_pending_callbacks;
        };

        template <>
        class FutureObject<void> : public FutureObjectInterface<void>,
                                   public EnableSharedFromThis<FutureObject<void> >,
                                   private FutureObjectBase {
        public:
            
            using typename FutureObjectInterface<void>::CallbackType;
            
            FutureObject() {}

            virtual ~FutureObject() {}
            
            virtual bool isDone() const
            {
                return FutureObjectBase::isDone();
            }
            
            virtual bool hasValue() const
            {
                return FutureObjectBase::hasValue();
            }

            virtual bool hasException() const
            {
                return FutureObjectBase::hasException();
            }
            
            virtual void get() const
            {
                wait();
            }

            virtual void set()
            {
                {
                    Mutex::Lock lock(m_mutex);
                    if (m_is_done) {
                        throwException(PromiseAlreadySatisfiedException(__FILE__, __LINE__));
                    }
                    markFinishedInternal();
                }

                doPendingCallbacks();
            }

            virtual void setException(const ExceptionPtr& e)
            {
                markFinishedWithException(e);
                doPendingCallbacks();
            }
            
            // pre: `callback' should not throw an exception because it may be run in
            // another thread at a later time so the exception can not be caught by the
            // caller and is meaningless. And this object must be shared by some
            // SharedPtr<FutureObject<void> > instance.
            void registerCallback(const CallbackType& callback)
            {
                WSD_ASSERT(callback);
                Mutex::Lock lock(m_mutex);
                if (m_is_done) {
                    lock.release();
                    try {
                        callback(this->sharedFromThis());
                    } catch (...) {
                        // Ignore the exceptions thrown by the callback.
                    }
                    return;
                }
                m_pending_callbacks.push_back(callback);
            }

        private:
            
            // Note: this function must be called without locks, otherwise deadlock will
            // happen because the callback passed by users may want to acquire the lock.
            void doPendingCallbacks()
            {
                std::list<CallbackType> callbacks;
                {
                    Mutex::Lock lock(m_mutex);
                    callbacks.swap(m_pending_callbacks);
                }

                for (std::list<CallbackType>::const_iterator it = callbacks.begin(),
                                                            end = callbacks.end();
                        it != end; ++it) {
                    (*it)(this->sharedFromThis());
                }
            }

            std::list<CallbackType> m_pending_callbacks;
        };

        template <typename R> class ForwardValue;
        
        // This class is used to wrap the callback passed by 'then()' so that its return
        // value can be captured and used to satisfy the promise that 'then()' returns'.
        template <typename R, typename T>
        class SequentialCallback {
        private:
            typedef typename detail::resolved_type<R>::type value_type;
            typedef SharedPtr<detail::FutureObjectInterface<T> > FuturePtr;
        public:
            SequentialCallback(const Callback<R(const Future<T>&)>& callback,
                               const Promise<value_type>& promise)
                : m_callback(callback),
                  m_promise(promise)
            {}
            
            // For callback which returns void.
            template <typename U>
            typename enable_if<is_void<U> >::type run(const FuturePtr& future)
            {
                try {
                    m_callback(future);
                    m_promise.set();
                } catch (...) {
                    m_promise.setException(currentException());
                }
            }

            // For callback which returns non-void non-future type
            template <typename U>
            typename enable_if_c<!is_void<U>::value && !is_future_type<U>::value>::type
            run(const FuturePtr& future)
            {
                try {
                    m_promise.setValue(m_callback(future));
                } catch (...) {
                    m_promise.setException(currentException());
                }
            }

            // For callback which returns future type.
            template <typename U>
            typename enable_if<is_future_type<U> >::type run(const FuturePtr& future)
            {
                try {
                    m_callback(future).then(bind(&ForwardValue<value_type>::template run<value_type>,
                                                 owned(new ForwardValue<value_type>(m_promise))));
                } catch (...) {
                    m_promise.setException(currentException());
                }
            }
            
        private:
            Callback<R(const Future<T>&)> m_callback;
            Promise<value_type> m_promise;
        };

        // When the callback passed by 'then()' returns a value of future type, this class
        // is used to capture the value from the promise satisfied by the callback and
        // foward it to the future that 'then()' returns.
        template <typename R>
        class ForwardValue {
        public:
            ForwardValue(const Promise<R>& promise)
                : m_promise(promise)
            {}

            template <typename V>
            typename enable_if<is_void<V> >::type run(const Future<V>& future)
            {
                try {
                    future.get();
                    m_promise.set();
                } catch (...) {
                    m_promise.setException(currentException());
                }
            }

            template <typename V>
            typename disable_if<is_void<V> >::type run(const Future<V>& future)
            {
                try {
                    m_promise.setValue(future.get());
                } catch (...) {
                    m_promise.setException(currentException());
                }
            }

        private:
            Promise<R> m_promise;
        };
        
        template <typename T>
        class FutureBase {
        private:
            typedef typename detail::FutureTraits<T>::move_dest_type move_dest_type;
            typedef typename detail::FutureTraits<T>::rvalue_source_type rvalue_source_type;
        public:
            FutureBase() {}

            FutureBase(const ExceptionPtr& e)
                : m_future(SharedPtr<detail::PromptFutureObject<T> >(new detail::PromptFutureObject<T>(e)))
            {}
        
            virtual ~FutureBase() {}

            move_dest_type get() const
            {
                if (!m_future)
                    throwException(FutureUninitialized(__FILE__, __LINE__));
                return m_future->get();
            }

            bool isDone() const
            {
                if (!m_future)
                    throwException(FutureUninitialized(__FILE__, __LINE__));
                return m_future && m_future->isDone();
            }

            bool hasValue() const
            {
                if (!m_future)
                    throwException(FutureUninitialized(__FILE__, __LINE__));
                return m_future && m_future->hasValue();
            }
        
            bool hasException() const
            {
                if (!m_future)
                    throwException(FutureUninitialized(__FILE__, __LINE__));
                return m_future && m_future->hasException();
            }
        
        protected:

            typedef SharedPtr<detail::FutureObjectInterface<T> > FuturePtr;
        
            FutureBase(const FuturePtr& future)
                : m_future(future)
            {}

            FuturePtr m_future;
        };

    }  // namespace detail

    template <typename T>
    class Future : public detail::FutureBase<T> {
    public:
        Future() {}

        Future(typename detail::FutureTraits<T>::rvalue_source_type t)
            : detail::FutureBase<T>(SharedPtr<detail::PromptFutureObject<T> >(new detail::PromptFutureObject<T>(t)))
        {}
            
        Future(const ExceptionPtr& e)
            : detail::FutureBase<T>(e)
        {}

        virtual ~Future() {}
        
        /**
         * Register a callback which will be called once the future is satisfied. If an
         * exception is thrown the callback will not be registered and then will not be called.
         * 
         * \throws std::bad_alloc if memory is unavailable.
         */
        template <typename R>
        Future<typename detail::resolved_type<R>::type> then(const Callback<R(const Future&)>& callback) const
        {
            typedef typename detail::resolved_type<R>::type value_type;

            if (!this->m_future)
                throwException(FutureUninitialized(__FILE__, __LINE__));

            Promise<value_type> promise;
            this->m_future->registerCallback(bind(&detail::SequentialCallback<R, T>::template run<R>,
                                                  owned(new detail::SequentialCallback<R, T>(callback, promise))));
            return promise.getFuture();
        }
        
    private:

        Future(const typename detail::FutureBase<T>::FuturePtr& future)
            : detail::FutureBase<T>(future)
        {}

        template <typename R, typename U> friend class detail::SequentialCallback;
        friend class Promise<T>;
    };

    template <>
    class Future<void> : public detail::FutureBase<void> {
    public:
        Future() {}

        Future(const ExceptionPtr& e)
            : detail::FutureBase<void>(e)
        {}

        virtual ~Future() {}
        
        /**
         * Register a callback which will be called once the future is satisfied. If an
         * exception is thrown the callback will not be registered and then will not be called.
         * 
         * \throws std::bad_alloc if memory is unavailable.
         */
        template <typename R>
        Future<typename detail::resolved_type<R>::type> then(const Callback<R(const Future&)>& callback) const
        {
            typedef typename detail::resolved_type<R>::type value_type;

            if (!this->m_future)
                throwException(FutureUninitialized(__FILE__, __LINE__));

            Promise<value_type> promise;
            this->m_future->registerCallback(bind(&detail::SequentialCallback<R, void>::template run<R>,
                                                  owned(new detail::SequentialCallback<R, void>(callback, promise))));
            return promise.getFuture();
        }
        
    private:

        Future(const detail::FutureBase<void>::FuturePtr& future)
            : detail::FutureBase<void>(future)
        {}

        /**
         * Creates a prompt future with a value satisfied.
         */
        friend Future<void> makeFuture();

        template <typename R, typename U> friend class detail::SequentialCallback;
        friend class Promise<void>;
    };

    template <typename T>
    typename disable_if<is_void<T>, Future<T> >::type
    makeFuture(typename detail::FutureTraits<T>::rvalue_source_type t)
    {
        return Future<T>(t);
    }

    inline Future<void> makeFuture()
    {
        return Future<void>(SharedPtr<detail::PromptFutureObject<void> >(new detail::PromptFutureObject<void>()));
    }

    template <typename T>
    class Promise {
    public:

        Promise()
            : m_future(SharedPtr<detail::FutureObject<T> >(new detail::FutureObject<T>()))
        {}
        
        /**
         * Satisfy the future with a value. If an exception is thrown the future is not satisfied.
         * 
         * \pre T must be CopyConstrutible.
         * \throws std::bad_alloc, PromiseAlreadySatisfiedException if the future has
         *     already been satisfied, or other exceptions thrown by T's copy constructor.
         */
        void setValue(typename detail::FutureTraits<T>::rvalue_source_type v)
        {
            WSD_ASSERT(m_future);
            m_future->setValue(v);
        }
                
        /**
         * Satisfy the future with an exception.
         *
         * \throws nothing
         */
        void setException(const ExceptionPtr& e)
        {
            m_future->setException(e);
        }

        /**
         * \throws nothing
         */
        Future<T> getFuture() const
        {
            return Future<T>(m_future);
        }

    private:
        SharedPtr<detail::FutureObjectInterface<T> > m_future;
    };

    template <>
    class Promise<void> {
    public:

        /**
         * \throws std::bad_alloc if memory is not available.
         */
        Promise()
            : m_future(SharedPtr<detail::FutureObject<void> >(new detail::FutureObject<void>()))
        {}
        
        void set()
        {
            WSD_ASSERT(m_future);
            m_future->set();
        }
                
        void setException(const ExceptionPtr& e)
        {
            m_future->setException(e);
        }

        Future<void> getFuture() const
        {
            return Future<void>(m_future);
        }
    private:
        SharedPtr<detail::FutureObjectInterface<void> > m_future;
    };

}  // namespace wsd

#endif  // __PROMISE_H__
