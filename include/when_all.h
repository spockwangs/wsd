// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __WHEN_ALL_H__
#define __WHEN_ALL_H__

#include <mutex>
#include <tuple>
#include <vector>

#include "promise.h"

namespace wsd {

namespace detail {

// Callback implementation for parallel composition of futures.

template <typename FutureAllValueType>
struct ParallelAllCallbackBase {
public:
    typedef Promise<FutureAllValueType> PromiseAll;

    ParallelAllCallbackBase(const PromiseAll& promise_all, size_t number_of_non_satisfied)
        : m_number_of_non_satisfied(number_of_non_satisfied), m_promise_all(promise_all)
    {
    }

    virtual ~ParallelAllCallbackBase()
    {
    }

    size_t m_number_of_non_satisfied;
    PromiseAll m_promise_all;
    FutureAllValueType m_waiting_futures;
    std::mutex m_mutex;

private:
    DISALLOW_COPY_AND_ASSIGN(ParallelAllCallbackBase);
};

// FutureAllValueType is supposed to be of type std::tuple<wsd::Future, ...> or
// std::vector<wsd::Future>.
//
// This implementation is for std::tuple<>.
template <typename FutureAllValueType>
class ParallelAllCallback : private ParallelAllCallbackBase<FutureAllValueType> {
public:
    typedef typename ParallelAllCallbackBase<FutureAllValueType>::PromiseAll PromiseAll;

    ParallelAllCallback(const PromiseAll& promise_all)
        : ParallelAllCallbackBase<FutureAllValueType>(promise_all, std::tuple_size<FutureAllValueType>::value)
    {
    }

    virtual ~ParallelAllCallback()
    {
    }

    template <int N>
    void on_future(const typename std::tuple_element<N, FutureAllValueType>::type& future)
    {
        // Note: this future must be recorded no matter what exception is thrown.
        std::unique_lock<std::mutex> lock(this->m_mutex);
        std::get<N>(this->m_waiting_futures) = future;
        assert(this->m_number_of_non_satisfied > 0);
        if (--this->m_number_of_non_satisfied == 0) {
            lock.unlock();
            try {
                this->m_promise_all.setValue(this->m_waiting_futures);
            } catch (...) {
                this->m_promise_all.setException(std::current_exception());
            }
            return;
        }
    }

private:
    DISALLOW_COPY_AND_ASSIGN(ParallelAllCallback);
};

// This implementation is specialized for std::vector<wsd::Future<> >.
template <typename FutureType>
class ParallelAllCallback<std::vector<FutureType>> : private ParallelAllCallbackBase<std::vector<FutureType>> {
public:
    typedef typename ParallelAllCallbackBase<std::vector<FutureType>>::PromiseAll PromiseAll;

    ParallelAllCallback(const PromiseAll& promise_all, size_t number_of_non_satisfied)
        : ParallelAllCallbackBase<std::vector<FutureType>>(promise_all, number_of_non_satisfied)
    {
        this->m_waiting_futures.resize(number_of_non_satisfied);
    }

    virtual ~ParallelAllCallback()
    {
    }

    void on_future(const FutureType& future, size_t idx)
    {
        // Note: this future must be recorded no matter what exception is thrown.
        assert(idx < this->m_waiting_futures.size());

        std::unique_lock<std::mutex> lock(this->m_mutex);
        this->m_waiting_futures[idx] = future;
        assert(this->m_number_of_non_satisfied > 0);
        if (--this->m_number_of_non_satisfied == 0) {
            lock.unlock();
            try {
                this->m_promise_all.setValue(this->m_waiting_futures);
            } catch (...) {
                this->m_promise_all.setException(std::current_exception());
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
    VectorParallelCallback(const std::shared_ptr<ParallelAllCallback<std::vector<Future<T>>>>& cb, size_t idx)
        : m_parallel_callback(cb), m_idx(idx)
    {
    }

    void on_future(const Future<T>& future)
    {
        m_parallel_callback->on_future(future, m_idx);
    }

private:
    std::shared_ptr<ParallelAllCallback<std::vector<Future<T>>>> m_parallel_callback;
    size_t m_idx;
};

}  // namespace detail

/**
 * Parallelly composite two or more futues. Only when all futures are satisfied is
 * the composited future satisfied.
 */
template <typename T1, typename T2>
Future<std::tuple<Future<T1>, Future<T2>>> whenAll(const Future<T1>& future1, const Future<T2>& future2)
{
    typedef std::tuple<Future<T1>, Future<T2>> FutureAllValueType;
    typedef Promise<FutureAllValueType> PromiseAll;
    typedef detail::ParallelAllCallback<FutureAllValueType> WhenAllCallback;

    PromiseAll promise_all;
    std::shared_ptr<WhenAllCallback> future_callback(new WhenAllCallback(promise_all));
    future1.then(wsd::bind(&WhenAllCallback::template on_future<0>, wsd::shared(future_callback)));
    future2.then(wsd::bind(&WhenAllCallback::template on_future<1>, wsd::shared(future_callback)));
    return promise_all.getFuture();
}

template <typename T1, typename T2, typename T3>
Future<std::tuple<Future<T1>, Future<T2>, Future<T3>>> whenAll(const Future<T1>& future1,
                                                               const Future<T2>& future2,
                                                               const Future<T3>& future3)
{
    typedef std::tuple<Future<T1>, Future<T2>, Future<T3>> FutureAllValueType;
    typedef Promise<FutureAllValueType> PromiseAll;
    typedef detail::ParallelAllCallback<FutureAllValueType> WhenAllCallback;

    PromiseAll promise_all;
    std::shared_ptr<WhenAllCallback> future_callback(new WhenAllCallback(promise_all));
    future1.then(wsd::bind(&WhenAllCallback::template on_future<0>, wsd::shared(future_callback)));
    future2.then(wsd::bind(&WhenAllCallback::template on_future<1>, wsd::shared(future_callback)));
    future3.then(wsd::bind(&WhenAllCallback::template on_future<2>, wsd::shared(future_callback)));
    return promise_all.getFuture();
}

template <typename T, typename InputIterator>
Future<std::vector<Future<T>>> whenAll(InputIterator first, InputIterator last)
{
    assert(first != last);

    typedef std::vector<Future<T>> FutureAllValueType;
    typedef Promise<FutureAllValueType> PromiseAll;
    typedef detail::ParallelAllCallback<FutureAllValueType> WhenAllCallback;

    PromiseAll promise_all;
    std::shared_ptr<WhenAllCallback> when_all_callback(new WhenAllCallback(promise_all, std::distance(first, last)));
    size_t i;

    for (i = 0; first != last; ++first, ++i) {
        first->then(wsd::bind(&detail::VectorParallelCallback<T>::on_future,
                              wsd::owned(new detail::VectorParallelCallback<T>(when_all_callback, i))));
    }
    return promise_all.getFuture();
}

}  // namespace wsd

#endif  // __WHEN_ALL_H__
