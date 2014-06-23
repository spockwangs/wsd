// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __WHEN_ALL_H__
#define __WHEN_ALL_H__

#include "promise.h"
#include <vector>
#include "boost/tuple/tuple.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/locks.hpp"
#include "boost/thread/lock_guard.hpp"
#include "boost/assert.hpp"

namespace wsd {
    
    namespace detail {

        // Callback implementation for parallel composition of futures.
        
        template <typename FutureAllValueType>
        struct ParallelAllCallbackBase {
        public:
            typedef Promise<FutureAllValueType> PromiseAll;
                
            ParallelAllCallbackBase(const PromiseAll& promise_all,
                                 size_t number_of_non_satisfied)
                : m_number_of_non_satisfied(number_of_non_satisfied),
                  m_promise_all(promise_all)
            {
            }

            virtual ~ParallelAllCallbackBase()
            {
            }

            size_t m_number_of_non_satisfied;
            PromiseAll m_promise_all;
            FutureAllValueType m_waiting_futures;
            boost::mutex m_mutex;

        private:
            DISALLOW_COPY_AND_ASSIGN(ParallelAllCallbackBase);
        };

        // FutureAllValueType is supposed to be of type boost::tuple<wsd::Future, ...> or
        // std::vector<wsd::Future>.
        //
        // This implementation is for boost::tuple<>.
        template <typename FutureAllValueType>
        class ParallelAllCallback : private ParallelAllCallbackBase<FutureAllValueType> {
        public:
            typedef typename ParallelAllCallbackBase<FutureAllValueType>::PromiseAll PromiseAll;
            
            ParallelAllCallback(const PromiseAll& promise_all)
                : ParallelAllCallbackBase<FutureAllValueType>(
                        promise_all, boost::tuples::length<FutureAllValueType>::value)
            {}

            virtual ~ParallelAllCallback() {}
        
            template <int N>
            void on_future(const typename boost::tuples::element<N, FutureAllValueType>::type& future)
            {
                // Note: this future must be recorded no matter what exception is thrown.
                boost::unique_lock<boost::mutex> lock(this->m_mutex);
                this->m_waiting_futures.template get<N>() = future;
                BOOST_ASSERT(this->m_number_of_non_satisfied > 0);
                if (--this->m_number_of_non_satisfied == 0) {
                    lock.unlock();
                    try {
                        this->m_promise_all.setValue(this->m_waiting_futures);
                    } catch (...) {
                        this->m_promise_all.setException(boost::current_exception());
                    }
                    return;
                }
            }

        private:
            DISALLOW_COPY_AND_ASSIGN(ParallelAllCallback);
        };

        // This implementation is specialized for std::vector<wsd::Future<> >.
        template <typename FutureType>
        class ParallelAllCallback<std::vector<FutureType> > : private ParallelAllCallbackBase<std::vector<FutureType> > {
        public:
            typedef typename ParallelAllCallbackBase<std::vector<FutureType> >::PromiseAll PromiseAll;
            
            ParallelAllCallback(const PromiseAll& promise_all,
                             size_t number_of_non_satisfied)
                : ParallelAllCallbackBase<std::vector<FutureType> >(promise_all, number_of_non_satisfied)
            {
                this->m_waiting_futures.resize(number_of_non_satisfied);
            }

            virtual ~ParallelAllCallback() {}
        
            void on_future(const FutureType& future, size_t idx)
            {
                // Note: this future must be recorded no matter what exception is thrown.
                BOOST_ASSERT(idx < this->m_waiting_futures.size());

                boost::unique_lock<boost::mutex> lock(this->m_mutex);
                this->m_waiting_futures[idx] = future;
                BOOST_ASSERT(this->m_number_of_non_satisfied > 0);
                if (--this->m_number_of_non_satisfied == 0) {
                    lock.unlock();
                    try {
                        this->m_promise_all.setValue(this->m_waiting_futures);
                    } catch (...) {
                        this->m_promise_all.setException(boost::current_exception());
                    }
                    return;
                }
            }
            
        private:
            DISALLOW_COPY_AND_ASSIGN(ParallelAllCallback);
        };

        // A helper class to notify ParallelAllCallback<std::vector<> > which future is done.
        template <typename T>
        class VectorParallelCallback {
        public:
            VectorParallelCallback(const boost::shared_ptr<ParallelAllCallback<std::vector<Future<T> > > >& cb, size_t idx)
                : m_parallel_callback(cb),
                  m_idx(idx)
            {}

            void on_future(const Future<T>& future)
            {
                m_parallel_callback->on_future(future, m_idx);
            }

        private:
            boost::shared_ptr<ParallelAllCallback<std::vector<Future<T> > > > m_parallel_callback;
            size_t m_idx;
        };
        
    }  // namespace detail
    
    /**
     * Parallelly composite two or more futues. Only when all futures are satisfied is
     * the composited future satisfied.
     */
    template <typename T1, typename T2>
    Future<boost::tuple<Future<T1>, Future<T2> > >
    whenAll(const Future<T1>& future1, const Future<T2>& future2)
    {
        typedef boost::tuple<Future<T1>, Future<T2> > FutureAllValueType;
        typedef Promise<FutureAllValueType> PromiseAll;
        typedef detail::ParallelAllCallback<FutureAllValueType> WhenAllCallback;

        PromiseAll promise_all;
        boost::shared_ptr<WhenAllCallback> future_callback(new WhenAllCallback(promise_all));
        future1.then(wsd::bind(&WhenAllCallback::template on_future<0>, wsd::shared(future_callback)));
        future2.then(wsd::bind(&WhenAllCallback::template on_future<1>, wsd::shared(future_callback)));
        return promise_all.getFuture();
    }

    template <typename T1, typename T2, typename T3>
    Future<boost::tuple<Future<T1>, Future<T2>, Future<T3> > >
    whenAll(const Future<T1>& future1, const Future<T2>& future2, const Future<T3>& future3)
    {
        typedef boost::tuple<Future<T1>, Future<T2>, Future<T3> > FutureAllValueType;
        typedef Promise<FutureAllValueType> PromiseAll;
        typedef detail::ParallelAllCallback<FutureAllValueType> WhenAllCallback;

        PromiseAll promise_all;
        boost::shared_ptr<WhenAllCallback> future_callback(new WhenAllCallback(promise_all));
        future1.then(wsd::bind(&WhenAllCallback::template on_future<0>, wsd::shared(future_callback)));
        future2.then(wsd::bind(&WhenAllCallback::template on_future<1>, wsd::shared(future_callback)));
        future3.then(wsd::bind(&WhenAllCallback::template on_future<2>, wsd::shared(future_callback)));
        return promise_all.getFuture();
    }

    template <typename T, typename InputIterator>
    Future<std::vector<Future<T> > >
    whenAll(InputIterator first, InputIterator last)
    {
        BOOST_ASSERT(first != last);
        
        typedef std::vector<Future<T> > FutureAllValueType;
        typedef Promise<FutureAllValueType> PromiseAll;
        typedef detail::ParallelAllCallback<FutureAllValueType> WhenAllCallback;

        PromiseAll promise_all;
        boost::shared_ptr<WhenAllCallback> when_all_callback(new WhenAllCallback(promise_all, std::distance(first, last)));
        size_t i;

        for (i = 0; first != last; ++first, ++i) {
            first->then(wsd::bind(&detail::VectorParallelCallback<T>::on_future,
                                  wsd::owned(new detail::VectorParallelCallback<T>(when_all_callback, i))));
        }
        return promise_all.getFuture();
    }
    
}  // namespace wsd

#endif  // __WHEN_ALL_H__
