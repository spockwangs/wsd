// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __PROMISE_H__
#define __PROMISE_H__

#include <cassert>
#include <exception>
#include <list>
#include <type_traits>
#include <mutex>
#include <condition_variable>
#include "callback.h"
#include "bind.h"
#include "wsd_magic.h"

namespace wsd {

class PromiseAlreadySatisfiedException : public std::exception {
};

class FutureUninitialized : public std::exception {

    };
    
    template <typename T> class Future;

    template <typename T> struct is_future_type : std::false_type {};
    template <typename T> struct is_future_type<Future<T> > : std::true_type {};

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
            // Use shared pointer to enable sharing (and avoid copying) of the resolved
            // value between two chained futures and improve the performance. See `copy()`
            // of `FutureTraits`, and its use in `ForwardValue` below.
            typedef std::shared_ptr<T> storage_type;
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

            static void copy(storage_type& dest, const storage_type& storage)
            {
                dest = storage;
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

            static void copy(storage_type& dest, const storage_type& storage)
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
            typedef typename FutureTraits<T>::storage_type storage_type;
            typedef Callback<void(const std::shared_ptr<FutureObjectInterface>&)> CallbackType;
            
            virtual ~FutureObjectInterface() {}

            virtual bool isDone() const = 0;
            virtual bool hasValue() const = 0;
            virtual bool hasException() const = 0;

            // Prompt futures will never use these.
            virtual void setValue(rvalue_source_type /*v*/) { assert(false); }
            virtual void setException(const std::exception_ptr& /*e*/) { assert(false); }

            virtual move_dest_type get() const = 0;
            virtual bool tryGet(dest_reference_type v) const = 0;
            virtual void registerCallback(const CallbackType& callback) = 0;
            virtual storage_type getStorageValue() const = 0;
            virtual void setValueFromStorage(const storage_type& /*v*/) { assert(false); }
        };
            
        template <>
        class FutureObjectInterface<void> {
        public:
            typedef FutureTraits<void>::move_dest_type move_dest_type;
            typedef Callback<void(const std::shared_ptr<FutureObjectInterface>&)> CallbackType;
            
            virtual ~FutureObjectInterface() {}

            virtual bool isDone() const = 0;
            virtual bool hasValue() const = 0;
            virtual bool hasException() const = 0;

            // Prompt futures will never use these.
            virtual void set() { assert(false); }
            virtual void setException(const std::exception_ptr& /*e*/) { assert(false); }

            virtual move_dest_type get() const = 0;
            virtual void registerCallback(const CallbackType& callback) = 0;
        };
        
        // Optimized implementation of FutureObjectInterface<> for prompt futures, which
        // does not require locking.
        //
        // See section 2.6 of [http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3327.pdf].
        template <typename T>
        class PromptFutureObject : public FutureObjectInterface<T>,
                                   public std::enable_shared_from_this<PromptFutureObject<T> > {
        public:
            typedef typename FutureObjectInterface<T>::move_dest_type move_dest_type;
            typedef typename FutureObjectInterface<T>::rvalue_source_type rvalue_source_type;
            typedef typename FutureObjectInterface<T>::dest_reference_type dest_reference_type;
            typedef typename FutureObjectInterface<T>::storage_type storage_type;
            typedef typename FutureObjectInterface<T>::CallbackType CallbackType;
            
            PromptFutureObject(rvalue_source_type v)
            {
                FutureTraits<T>::init(m_value, v);
            }

            PromptFutureObject(const std::exception_ptr& e)
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
            
            virtual bool hasException() const { return m_exception_ptr != nullptr; }
            
            virtual move_dest_type get() const
            {
                assert(m_exception_ptr || m_value);
                if (m_exception_ptr)
                    std::rethrow_exception(m_exception_ptr);
                return *m_value;
            }
            
            virtual bool tryGet(dest_reference_type v) const
            {
                FutureTraits<T>::assign(v, m_value);
                return true;
            }
            
            virtual void registerCallback(const CallbackType& callback)
            {
                assert(callback);
                try {
                    callback(this->shared_from_this());
                } catch (...) {
                    // Ingore the exception thrown by the callback.
                }
            }
            
            virtual storage_type getStorageValue() const
            {
                return m_value;
            }
            
        private:
            typename FutureTraits<T>::storage_type m_value;
            std::exception_ptr m_exception_ptr;
        };
        
        template <>
        class PromptFutureObject<void> : public FutureObjectInterface<void>,
                                         public std::enable_shared_from_this<PromptFutureObject<void> > {
        public:
            typedef FutureObjectInterface<void>::move_dest_type move_dest_type;
            typedef FutureObjectInterface<void>::CallbackType CallbackType;
            
            PromptFutureObject() {}

            PromptFutureObject(const std::exception_ptr& e)
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
            
            virtual bool hasException() const { return m_exception_ptr != nullptr; }

            // Promise<> will never this implementation.
            virtual void set()
            {
                assert(false);
            }
            
            virtual void setException(const std::exception_ptr& /*e*/)
            {
                assert(false);
            }
            
            virtual move_dest_type get() const
            {
                if (m_exception_ptr)
                    std::rethrow_exception(m_exception_ptr);
            }
            
            virtual void registerCallback(const CallbackType& callback)
            {
                assert(callback);
                try {
                    callback(this->shared_from_this());
                } catch (...) {
                    // Ingore the exception thrown by the callback.
                }
            }
            
        private:
            std::exception_ptr m_exception_ptr;
        };

        struct FutureObjectBase {
            FutureObjectBase()
                : m_is_done(false)
            {}

            virtual ~FutureObjectBase() {}
            
            bool isDone() const
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                return m_is_done;
            }
            
            bool hasValue() const
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                return m_is_done && !m_exception_ptr;
            }

            bool hasException() const
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                return m_is_done && m_exception_ptr;
            }
            
            void markFinishedWithException(const std::exception_ptr& e)
            {
                assert(e);
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_is_done) {
                    throw PromiseAlreadySatisfiedException();
                }
                m_exception_ptr = e;
                markFinishedInternal();
            }

            void markFinishedInternal()
            {
                m_is_done = true;
                m_cond.notify_all();
            }

            void wait() const
            {
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    while (!m_is_done) {
                        // This promise is not satisfied yet. So we wait.
                        m_cond.wait(lock);
                    }
                }

                if (m_exception_ptr)
                    std::rethrow_exception(m_exception_ptr);
            }

            mutable std::mutex m_mutex;
            mutable std::condition_variable m_cond;  // predicate: m_is_done == true
            bool m_is_done;                    // either a value or exception is set
            std::exception_ptr m_exception_ptr;
        };
        
        template <typename T>
        class FutureObject : public FutureObjectInterface<T>,
                             public std::enable_shared_from_this<FutureObject<T> >,
                             private FutureObjectBase {
        public:
            typedef typename FutureObjectInterface<T>::move_dest_type move_dest_type;
            typedef typename FutureObjectInterface<T>::rvalue_source_type rvalue_source_type;
            typedef typename FutureObjectInterface<T>::storage_type storage_type;
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

                assert(m_value);
                return *m_value;
            }

            virtual bool tryGet(dest_reference_type v) const
            {
                if (!isDone())
                    return false;
                if (m_exception_ptr)
                    std::rethrow_exception(m_exception_ptr);
                FutureTraits<T>::assign(v, m_value);
                return true;
            }
            
            virtual void setException(const std::exception_ptr& e)
            {
                markFinishedWithException(e);
                doPendingCallbacks();
            }
            
            virtual void setValue(rvalue_source_type t)
            {
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (m_is_done) {
                        throw PromiseAlreadySatisfiedException();
                    }
                    FutureTraits<T>::init(m_value, t);
                    markFinishedInternal();
                }
                doPendingCallbacks();
            }
            
            // pre: `callback' should not throw an exception because it may be run in
            // another thread at a later time so the exception can not be caught by the
            // caller and is meaningless. And this object must be shared by some
            // std::shared_ptr<FutureObject<T> > instance.
            virtual void registerCallback(const CallbackType& callback)
            {
                assert(callback);
                std::unique_lock<std::mutex> lock(m_mutex);
                if (m_is_done) {
                    lock.unlock();
                    try {
                        callback(this->shared_from_this());
                    } catch (...) {
                        // Ignore the exceptions thrown by the callback.
                    }
                } else {
                    m_pending_callbacks.push_back(callback);
                }
            }

            virtual storage_type getStorageValue() const
            {
                wait();

                assert(m_value);
                return m_value;
            }

            virtual void setValueFromStorage(const storage_type& v)
            {
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (m_is_done) {
                        throw PromiseAlreadySatisfiedException();
                    }
                    FutureTraits<T>::copy(m_value, v);
                    markFinishedInternal();
                }
                doPendingCallbacks();
            }
            
        private:

            // Note: this function must be called without locks, otherwise deadlock will
            // happen because the callback passed by users may want to acquire the lock.
            void doPendingCallbacks()
            {
                std::list<CallbackType> callbacks;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    callbacks.swap(m_pending_callbacks);
                }

                for (typename std::list<CallbackType>::const_iterator it = callbacks.begin(),
                                                                     end = callbacks.end();
                        it != end; ++it) {
                    try {
                        (*it)(this->shared_from_this());
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
                                   public std::enable_shared_from_this<FutureObject<void> >,
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
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (m_is_done) {
                        throw PromiseAlreadySatisfiedException();
                    }
                    markFinishedInternal();
                }

                doPendingCallbacks();
            }

            virtual void setException(const std::exception_ptr& e)
            {
                markFinishedWithException(e);
                doPendingCallbacks();
            }
            
            // pre: `callback' should not throw an exception because it may be run in
            // another thread at a later time so the exception can not be caught by the
            // caller and is meaningless. And this object must be shared by some
            // std::shared_ptr<FutureObject<void> > instance.
            void registerCallback(const CallbackType& callback)
            {
                assert(callback);
                std::unique_lock<std::mutex> lock(m_mutex);
                if (m_is_done) {
                    lock.unlock();
                    try {
                        callback(this->shared_from_this());
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
                    std::lock_guard<std::mutex> lock(m_mutex);
                    callbacks.swap(m_pending_callbacks);
                }

                for (std::list<CallbackType>::const_iterator it = callbacks.begin(),
                                                            end = callbacks.end();
                        it != end; ++it) {
                    (*it)(this->shared_from_this());
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
            typedef std::shared_ptr<detail::FutureObjectInterface<T> > FuturePtr;
        public:
            SequentialCallback(const Callback<R(const Future<T>&)>& callback,
                               const Promise<value_type>& promise)
                : m_callback(callback),
                  m_promise(promise)
            {}
            
            // For callback which returns void.
            template <typename U>
            typename std::enable_if<std::is_void<U>::value>::type run(const FuturePtr& future)
            {
                try {
                    m_callback(future);
                    m_promise.set();
                } catch (...) {
                    m_promise.setException(std::current_exception());
                }
            }

            // For callback which returns non-void non-future type
            template <typename U>
            typename std::enable_if<!std::is_void<U>::value && !is_future_type<U>::value>::type
            run(const FuturePtr& future)
            {
                try {
                    m_promise.setValue(m_callback(future));
                } catch (...) {
                    m_promise.setException(std::current_exception());
                }
            }

            // For callback which returns future type.
            template <typename U>
            typename std::enable_if<is_future_type<U>::value>::type run(const FuturePtr& future)
            {
                try {
                    m_callback(future).then(wsd::bind(&ForwardValue<value_type>::template run<value_type>,
                                                 owned(new ForwardValue<value_type>(m_promise))));
                } catch (...) {
                    m_promise.setException(std::current_exception());
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
            typename std::enable_if<std::is_void<V>::value>::type run(const Future<V>& future)
            {
                try {
                    future.get();
                    m_promise.set();
                } catch (...) {
                    m_promise.setException(std::current_exception());
                }
            }

            template <typename V>
            typename std::enable_if<!std::is_void<V>::value>::type run(const Future<V>& future)
            {
                try {
                    m_promise.m_future->setValueFromStorage(future.m_future->getStorageValue());
                } catch (...) {
                    m_promise.setException(std::current_exception());
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
            typedef bool (FutureBase::*unspecified_bool_type)() const;
            
            FutureBase() {}

            FutureBase(const std::exception_ptr& e)
                : m_future(std::shared_ptr<detail::PromptFutureObject<T> >(new detail::PromptFutureObject<T>(e)))
            {}
        
            virtual ~FutureBase() {}

            move_dest_type get() const
            {
                if (!m_future)
                    throw FutureUninitialized();
                return m_future->get();
            }

            bool isDone() const
            {
                return m_future && m_future->isDone();
            }

            bool hasValue() const
            {
                return m_future && m_future->hasValue();
            }
        
            bool hasException() const
            {
                return m_future && m_future->hasException();
            }
        
            /**
             * Returns true if this future has been initialized.
             */
            operator unspecified_bool_type() const
            {
                return m_future ? &FutureBase::isDone : NULL;
            }

        protected:
            typedef std::shared_ptr<detail::FutureObjectInterface<T> > FuturePtr;
        
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
            : detail::FutureBase<T>(std::shared_ptr<detail::PromptFutureObject<T> >(new detail::PromptFutureObject<T>(t)))
        {}
            
        Future(const std::exception_ptr& e)
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
                throw FutureUninitialized();

            Promise<value_type> promise;
            this->m_future->registerCallback(wsd::bind(&detail::SequentialCallback<R, T>::template run<R>,
                                                  owned(new detail::SequentialCallback<R, T>(callback, promise))));
            return promise.getFuture();
        }
        
    private:

        Future(const typename detail::FutureBase<T>::FuturePtr& future)
            : detail::FutureBase<T>(future)
        {}

        template <typename R, typename U> friend class detail::SequentialCallback;
        template <typename R> friend class detail::ForwardValue;
        friend class Promise<T>;
    };

    template <>
    class Future<void> : public detail::FutureBase<void> {
    public:
        Future() {}

        Future(const std::exception_ptr& e)
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
                throw FutureUninitialized();

            Promise<value_type> promise;
            this->m_future->registerCallback(wsd::bind(&detail::SequentialCallback<R, void>::template run<R>,
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
    typename std::enable_if<!std::is_void<T>::value, Future<T> >::type
    makeFuture(typename detail::FutureTraits<T>::rvalue_source_type t)
    {
        return Future<T>(t);
    }

    inline Future<void> makeFuture()
    {
        return Future<void>(std::shared_ptr<detail::PromptFutureObject<void> >(
                                    new detail::PromptFutureObject<void>()));
    }

    template <typename T>
    class Promise {
    public:

        Promise()
            : m_future(std::shared_ptr<detail::FutureObject<T> >(new detail::FutureObject<T>()))
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
            assert(m_future);
            m_future->setValue(v);
        }
                
        /**
         * Satisfy the future with an exception.
         *
         * \throws nothing
         */
        void setException(const std::exception_ptr& e)
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
        template <typename R> friend class detail::ForwardValue;

        std::shared_ptr<detail::FutureObjectInterface<T> > m_future;
    };

    template <>
    class Promise<void> {
    public:

        /**
         * \throws std::bad_alloc if memory is not available.
         */
        Promise()
            : m_future(std::shared_ptr<detail::FutureObject<void> >(new detail::FutureObject<void>()))
        {}
        
        void set()
        {
            assert(m_future);
            m_future->set();
        }
                
        void setException(const std::exception_ptr& e)
        {
            m_future->setException(e);
        }

        Future<void> getFuture() const
        {
            return Future<void>(m_future);
        }
    private:
        std::shared_ptr<detail::FutureObjectInterface<void> > m_future;
    };

}  // namespace wsd

#endif  // __PROMISE_H__
