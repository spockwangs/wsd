#ifndef __BIND_H__
#define __BIND_H__

#include "callback.h"
#include "bind_detail.h"
#include "boost/static_assert.hpp"
#include "boost/type_traits/is_pointer.hpp"

namespace wsd {

    template <typename Functor>
    Callback<typename detail::BindState<
                 typename detail::FunctorTraits<Functor>::FunctorType,
                 typename detail::FunctorTraits<Functor>::RunType,
                 void ()>::UnboundRunType>
    bind(Functor functor)
    {
        typedef typename detail::FunctorTraits<Functor>::RunType RunType;
        typedef typename detail::FunctorTraits<Functor>::FunctorType FunctorType;
        typedef detail::BindState<FunctorType, RunType, void()> BindState;
        
        return Callback<typename BindState::UnboundRunType>(new BindState(FunctorType(functor)));
    }

    template <typename Functor, typename A1>
    Callback<typename detail::BindState<
                 typename detail::FunctorTraits<Functor>::FunctorType,
                 typename detail::FunctorTraits<Functor>::RunType,
                 void (typename detail::CallbackParamTraits<A1>::StorageType)
                 >::UnboundRunType>
    bind(Functor functor, const A1& a1)
    {
        typedef typename detail::FunctorTraits<Functor>::RunType RunType;
        typedef typename detail::FunctorTraits<Functor>::FunctorType FunctorType;

        // Do not allow non-const reference for the parameters of RunType. Binding
        // a non-const reference parameter can make for subtle bugs because the invoked
        // function will receive a reference to the stored copy of the argument and not
        // the original. 
        BOOST_STATIC_ASSERT(
                !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A1Type>::value);

        // Do not allow pass raw pointers as the bound argument, because this may cause
        // memory leak. Use wsd::unretained(), wsd::owned() to specify the memory
        // ownership explicitly.
        BOOST_STATIC_ASSERT_MSG(!::boost::is_pointer<A1>::value, "do_not_pass_raw_pointers_as_bound_argument");
        
        typedef detail::BindState<FunctorType, RunType,
                                  void(typename detail::CallbackParamTraits<A1>::StorageType)
                                  > BindState;
        
        return Callback<typename BindState::UnboundRunType>(new BindState(FunctorType(functor), a1));
    }

    template <typename Functor, typename A1, typename A2>
    Callback<typename detail::BindState<
                 typename detail::FunctorTraits<Functor>::FunctorType,
                 typename detail::FunctorTraits<Functor>::RunType,
                 void (typename detail::CallbackParamTraits<A1>::StorageType,
                       typename detail::CallbackParamTraits<A2>::StorageType)
                 >::UnboundRunType>
    bind(Functor functor, const A1& a1, const A2& a2)
    {
        typedef typename detail::FunctorTraits<Functor>::RunType RunType;
        typedef typename detail::FunctorTraits<Functor>::FunctorType FunctorType;

        // Do not allow non-const reference for the parameters of RunType. Binding
        // a non-const reference parameter can make for subtle bugs because the invoked
        // function will receive a reference to the stored copy of the argument and not
        // the original. 
        BOOST_STATIC_ASSERT_MSG(!detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A1Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A2Type>::value,
                      do_not_bind_functions_with_non_const_ref);

        // Do not allow pass raw pointers as the bound argument, because this may cause
        // memory leak. Use wsd::unretained(), wsd::owned() to specify the memory
        // ownership explicitly.
        BOOST_STATIC_ASSERT_MSG(!::boost::is_pointer<A1>::value
                      && !::boost::is_pointer<A2>::value,
                      do_not_pass_raw_pointers_as_bound_argument);
        
        typedef detail::BindState<FunctorType, RunType,
                                  void(typename detail::CallbackParamTraits<A1>::StorageType,
                                       typename detail::CallbackParamTraits<A2>::StorageType)
                                  > BindState;
        
        return Callback<typename BindState::UnboundRunType>(new BindState(FunctorType(functor), a1, a2));
    }

    template <typename Functor, typename A1, typename A2, typename A3>
    Callback<typename detail::BindState<
                 typename detail::FunctorTraits<Functor>::FunctorType,
                 typename detail::FunctorTraits<Functor>::RunType,
                 void (typename detail::CallbackParamTraits<A1>::StorageType,
                       typename detail::CallbackParamTraits<A2>::StorageType,
                       typename detail::CallbackParamTraits<A3>::StorageType)
                 >::UnboundRunType>
    bind(Functor functor, const A1& a1, const A2& a2, const A3& a3)
    {
        typedef typename detail::FunctorTraits<Functor>::RunType RunType;
        typedef typename detail::FunctorTraits<Functor>::FunctorType FunctorType;

        // Do not allow non-const reference for the parameters of RunType. Binding
        // a non-const reference parameter can make for subtle bugs because the invoked
        // function will receive a reference to the stored copy of the argument and not
        // the original. 
        BOOST_STATIC_ASSERT_MSG(!detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A1Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A2Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A3Type>::value,
                      do_not_bind_functions_with_non_const_ref);

        // Do not allow pass raw pointers as the bound argument, because this may cause
        // memory leak. Use wsd::unretained(), wsd::owned() to specify the memory
        // ownership explicitly.
        BOOST_STATIC_ASSERT_MSG(!::boost::is_pointer<A1>::value
                      && !::boost::is_pointer<A2>::value
                      && !::boost::is_pointer<A3>::value,
                      do_not_pass_raw_pointers_as_bound_argument);
        
        typedef detail::BindState<FunctorType, RunType,
                                  void(typename detail::CallbackParamTraits<A1>::StorageType,
                                       typename detail::CallbackParamTraits<A2>::StorageType,
                                       typename detail::CallbackParamTraits<A3>::StorageType)
                                  > BindState;
        
        return Callback<typename BindState::UnboundRunType>(new BindState(FunctorType(functor), a1, a2, a3));
    }

    template <typename Functor, typename A1, typename A2, typename A3, typename A4>
    Callback<typename detail::BindState<
                 typename detail::FunctorTraits<Functor>::FunctorType,
                 typename detail::FunctorTraits<Functor>::RunType,
                 void (typename detail::CallbackParamTraits<A1>::StorageType,
                       typename detail::CallbackParamTraits<A2>::StorageType,
                       typename detail::CallbackParamTraits<A3>::StorageType,
                       typename detail::CallbackParamTraits<A4>::StorageType)
                 >::UnboundRunType>
    bind(Functor functor, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
    {
        typedef typename detail::FunctorTraits<Functor>::RunType RunType;
        typedef typename detail::FunctorTraits<Functor>::FunctorType FunctorType;

        // Do not allow non-const reference for the parameters of RunType. Binding
        // a non-const reference parameter can make for subtle bugs because the invoked
        // function will receive a reference to the stored copy of the argument and not
        // the original. 
        BOOST_STATIC_ASSERT_MSG(!detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A1Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A2Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A3Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A4Type>::value,
                      do_not_bind_functions_with_non_const_ref);

        // Do not allow pass raw pointers as the bound argument, because this may cause
        // memory leak. Use wsd::unretained(), wsd::owned() to specify the memory
        // ownership explicitly.
        BOOST_STATIC_ASSERT_MSG(!::boost::is_pointer<A1>::value
                      && !::boost::is_pointer<A2>::value
                      && !::boost::is_pointer<A3>::value
                      && !::boost::is_pointer<A4>::value,
                      do_not_pass_raw_pointers_as_bound_argument);
        
        typedef detail::BindState<FunctorType, RunType,
                                  void(typename detail::CallbackParamTraits<A1>::StorageType,
                                       typename detail::CallbackParamTraits<A2>::StorageType,
                                       typename detail::CallbackParamTraits<A3>::StorageType,
                                       typename detail::CallbackParamTraits<A4>::StorageType)
                                  > BindState;
        
        return Callback<typename BindState::UnboundRunType>(new BindState(FunctorType(functor), a1, a2, a3, a4));
    }

    template <typename Functor, typename A1, typename A2, typename A3, typename A4, typename A5>
    Callback<typename detail::BindState<
                 typename detail::FunctorTraits<Functor>::FunctorType,
                 typename detail::FunctorTraits<Functor>::RunType,
                 void (typename detail::CallbackParamTraits<A1>::StorageType,
                       typename detail::CallbackParamTraits<A2>::StorageType,
                       typename detail::CallbackParamTraits<A3>::StorageType,
                       typename detail::CallbackParamTraits<A4>::StorageType,
                       typename detail::CallbackParamTraits<A5>::StorageType)
                 >::UnboundRunType>
    bind(Functor functor, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
    {
        typedef typename detail::FunctorTraits<Functor>::RunType RunType;
        typedef typename detail::FunctorTraits<Functor>::FunctorType FunctorType;

        // Do not allow non-const reference for the parameters of RunType. Binding
        // a non-const reference parameter can make for subtle bugs because the invoked
        // function will receive a reference to the stored copy of the argument and not
        // the original. 
        BOOST_STATIC_ASSERT_MSG(!detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A1Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A2Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A3Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A4Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A5Type>::value,
                      do_not_bind_functions_with_non_const_ref);

        // Do not allow pass raw pointers as the bound argument, because this may cause
        // memory leak. Use wsd::unretained(), wsd::owned() to specify the memory
        // ownership explicitly.
        BOOST_STATIC_ASSERT_MSG(!::boost::is_pointer<A1>::value
                      && !::boost::is_pointer<A2>::value
                      && !::boost::is_pointer<A3>::value
                      && !::boost::is_pointer<A4>::value
                      && !::boost::is_pointer<A5>::value,
                      do_not_pass_raw_pointers_as_bound_argument);
        
        typedef detail::BindState<FunctorType, RunType,
                                  void(typename detail::CallbackParamTraits<A1>::StorageType,
                                       typename detail::CallbackParamTraits<A2>::StorageType,
                                       typename detail::CallbackParamTraits<A3>::StorageType,
                                       typename detail::CallbackParamTraits<A4>::StorageType,
                                       typename detail::CallbackParamTraits<A5>::StorageType)
                                  > BindState;
        
        return Callback<typename BindState::UnboundRunType>(new BindState(FunctorType(functor), a1, a2, a3, a4, a5));
    }

    template <typename Functor, typename A1, typename A2, typename A3, typename A4,
              typename A5, typename A6>
    Callback<typename detail::BindState<
                 typename detail::FunctorTraits<Functor>::FunctorType,
                 typename detail::FunctorTraits<Functor>::RunType,
                 void (typename detail::CallbackParamTraits<A1>::StorageType,
                       typename detail::CallbackParamTraits<A2>::StorageType,
                       typename detail::CallbackParamTraits<A3>::StorageType,
                       typename detail::CallbackParamTraits<A4>::StorageType,
                       typename detail::CallbackParamTraits<A5>::StorageType,
                       typename detail::CallbackParamTraits<A6>::StorageType)
                 >::UnboundRunType>
    bind(Functor functor, const A1& a1, const A2& a2, const A3& a3, const A4& a4,
         const A5& a5, const A6& a6)
    {
        typedef typename detail::FunctorTraits<Functor>::RunType RunType;
        typedef typename detail::FunctorTraits<Functor>::FunctorType FunctorType;

        // Do not allow non-const reference for the parameters of RunType. Binding
        // a non-const reference parameter can make for subtle bugs because the invoked
        // function will receive a reference to the stored copy of the argument and not
        // the original. 
        BOOST_STATIC_ASSERT_MSG(!detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A1Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A2Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A3Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A4Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A5Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A6Type>::value,
                      do_not_bind_functions_with_non_const_ref);

        // Do not allow pass raw pointers as the bound argument, because this may cause
        // memory leak. Use wsd::unretained(), wsd::owned() to specify the memory
        // ownership explicitly.
        BOOST_STATIC_ASSERT_MSG(!::boost::is_pointer<A1>::value
                      && !::boost::is_pointer<A2>::value
                      && !::boost::is_pointer<A3>::value
                      && !::boost::is_pointer<A4>::value
                      && !::boost::is_pointer<A5>::value
                      && !::boost::is_pointer<A6>::value,
                      do_not_pass_raw_pointers_as_bound_argument);
        
        typedef detail::BindState<FunctorType, RunType,
                                  void(typename detail::CallbackParamTraits<A1>::StorageType,
                                       typename detail::CallbackParamTraits<A2>::StorageType,
                                       typename detail::CallbackParamTraits<A3>::StorageType,
                                       typename detail::CallbackParamTraits<A4>::StorageType,
                                       typename detail::CallbackParamTraits<A5>::StorageType,
                                       typename detail::CallbackParamTraits<A6>::StorageType)
                                  > BindState;
        
        return Callback<typename BindState::UnboundRunType>(
                new BindState(FunctorType(functor), a1, a2, a3, a4, a5, a6));
    }

    template <typename Functor, typename A1, typename A2, typename A3, typename A4,
              typename A5, typename A6, typename A7>
    Callback<typename detail::BindState<
                 typename detail::FunctorTraits<Functor>::FunctorType,
                 typename detail::FunctorTraits<Functor>::RunType,
                 void (typename detail::CallbackParamTraits<A1>::StorageType,
                       typename detail::CallbackParamTraits<A2>::StorageType,
                       typename detail::CallbackParamTraits<A3>::StorageType,
                       typename detail::CallbackParamTraits<A4>::StorageType,
                       typename detail::CallbackParamTraits<A5>::StorageType,
                       typename detail::CallbackParamTraits<A6>::StorageType,
                       typename detail::CallbackParamTraits<A7>::StorageType)
                 >::UnboundRunType>
    bind(Functor functor, const A1& a1, const A2& a2, const A3& a3, const A4& a4,
         const A5& a5, const A6& a6, const A7& a7)
    {
        typedef typename detail::FunctorTraits<Functor>::RunType RunType;
        typedef typename detail::FunctorTraits<Functor>::FunctorType FunctorType;

        // Do not allow non-const reference for the parameters of RunType. Binding
        // a non-const reference parameter can make for subtle bugs because the invoked
        // function will receive a reference to the stored copy of the argument and not
        // the original. 
        BOOST_STATIC_ASSERT_MSG(!detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A1Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A2Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A3Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A4Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A5Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A6Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A7Type>::value,
                      do_not_bind_functions_with_non_const_ref);

        // Do not allow pass raw pointers as the bound argument, because this may cause
        // memory leak. Use wsd::unretained(), wsd::owned() to specify the memory
        // ownership explicitly.
        BOOST_STATIC_ASSERT_MSG(!::boost::is_pointer<A1>::value
                      && !::boost::is_pointer<A2>::value
                      && !::boost::is_pointer<A3>::value
                      && !::boost::is_pointer<A4>::value
                      && !::boost::is_pointer<A5>::value
                      && !::boost::is_pointer<A6>::value
                      && !::boost::is_pointer<A7>::value,
                      do_not_pass_raw_pointers_as_bound_argument);
        
        typedef detail::BindState<FunctorType, RunType,
                                  void(typename detail::CallbackParamTraits<A1>::StorageType,
                                       typename detail::CallbackParamTraits<A2>::StorageType,
                                       typename detail::CallbackParamTraits<A3>::StorageType,
                                       typename detail::CallbackParamTraits<A4>::StorageType,
                                       typename detail::CallbackParamTraits<A5>::StorageType,
                                       typename detail::CallbackParamTraits<A6>::StorageType,
                                       typename detail::CallbackParamTraits<A7>::StorageType)
                                  > BindState;
        
        return Callback<typename BindState::UnboundRunType>(
                new BindState(FunctorType(functor), a1, a2, a3, a4, a5, a6, a7));
    }

    template <typename Functor, typename A1, typename A2, typename A3, typename A4,
              typename A5, typename A6, typename A7, typename A8>
    Callback<typename detail::BindState<
                 typename detail::FunctorTraits<Functor>::FunctorType,
                 typename detail::FunctorTraits<Functor>::RunType,
                 void (typename detail::CallbackParamTraits<A1>::StorageType,
                       typename detail::CallbackParamTraits<A2>::StorageType,
                       typename detail::CallbackParamTraits<A3>::StorageType,
                       typename detail::CallbackParamTraits<A4>::StorageType,
                       typename detail::CallbackParamTraits<A5>::StorageType,
                       typename detail::CallbackParamTraits<A6>::StorageType,
                       typename detail::CallbackParamTraits<A7>::StorageType,
                       typename detail::CallbackParamTraits<A8>::StorageType)
                 >::UnboundRunType>
    bind(Functor functor, const A1& a1, const A2& a2, const A3& a3, const A4& a4,
         const A5& a5, const A6& a6, const A7& a7, const A8& a8)
    {
        typedef typename detail::FunctorTraits<Functor>::RunType RunType;
        typedef typename detail::FunctorTraits<Functor>::FunctorType FunctorType;

        // Do not allow non-const reference for the parameters of RunType. Binding
        // a non-const reference parameter can make for subtle bugs because the invoked
        // function will receive a reference to the stored copy of the argument and not
        // the original. 
        BOOST_STATIC_ASSERT_MSG(!detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A1Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A2Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A3Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A4Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A5Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A6Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A7Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A8Type>::value,
                      do_not_bind_functions_with_non_const_ref);

        // Do not allow pass raw pointers as the bound argument, because this may cause
        // memory leak. Use wsd::unretained(), wsd::owned() to specify the memory
        // ownership explicitly.
        BOOST_STATIC_ASSERT_MSG(!::boost::is_pointer<A1>::value
                      && !::boost::is_pointer<A2>::value
                      && !::boost::is_pointer<A3>::value
                      && !::boost::is_pointer<A4>::value
                      && !::boost::is_pointer<A5>::value
                      && !::boost::is_pointer<A6>::value
                      && !::boost::is_pointer<A7>::value
                      && !::boost::is_pointer<A8>::value,
                      do_not_pass_raw_pointers_as_bound_argument);
        
        typedef detail::BindState<FunctorType, RunType,
                                  void(typename detail::CallbackParamTraits<A1>::StorageType,
                                       typename detail::CallbackParamTraits<A2>::StorageType,
                                       typename detail::CallbackParamTraits<A3>::StorageType,
                                       typename detail::CallbackParamTraits<A4>::StorageType,
                                       typename detail::CallbackParamTraits<A5>::StorageType,
                                       typename detail::CallbackParamTraits<A6>::StorageType,
                                       typename detail::CallbackParamTraits<A7>::StorageType,
                                       typename detail::CallbackParamTraits<A8>::StorageType)
                                  > BindState;
        
        return Callback<typename BindState::UnboundRunType>(
                new BindState(FunctorType(functor), a1, a2, a3, a4, a5, a6, a7, a8));
    }

    template <typename Functor, typename A1, typename A2, typename A3, typename A4,
              typename A5, typename A6, typename A7, typename A8, typename A9>
    Callback<typename detail::BindState<
                 typename detail::FunctorTraits<Functor>::FunctorType,
                 typename detail::FunctorTraits<Functor>::RunType,
                 void (typename detail::CallbackParamTraits<A1>::StorageType,
                       typename detail::CallbackParamTraits<A2>::StorageType,
                       typename detail::CallbackParamTraits<A3>::StorageType,
                       typename detail::CallbackParamTraits<A4>::StorageType,
                       typename detail::CallbackParamTraits<A5>::StorageType,
                       typename detail::CallbackParamTraits<A6>::StorageType,
                       typename detail::CallbackParamTraits<A7>::StorageType,
                       typename detail::CallbackParamTraits<A8>::StorageType,
                       typename detail::CallbackParamTraits<A9>::StorageType)
                 >::UnboundRunType>
    bind(Functor functor, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9)
    {
        typedef typename detail::FunctorTraits<Functor>::RunType RunType;
        typedef typename detail::FunctorTraits<Functor>::FunctorType FunctorType;

        // Do not allow non-const reference for the parameters of RunType. Binding
        // a non-const reference parameter can make for subtle bugs because the invoked
        // function will receive a reference to the stored copy of the argument and not
        // the original. 
        BOOST_STATIC_ASSERT_MSG(!detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A1Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A2Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A3Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A4Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A5Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A6Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A7Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A8Type>::value
                      && !detail::is_non_const_reference<typename detail::FunctionTraits<RunType>::A9Type>::value,
                      do_not_bind_functions_with_non_const_ref);

        // Do not allow pass raw pointers as the bound argument, because this may cause
        // memory leak. Use wsd::unretained(), wsd::owned() to specify the memory
        // ownership explicitly.
        BOOST_STATIC_ASSERT_MSG(!::boost::is_pointer<A1>::value
                      && !::boost::is_pointer<A2>::value
                      && !::boost::is_pointer<A3>::value
                      && !::boost::is_pointer<A4>::value
                      && !::boost::is_pointer<A5>::value
                      && !::boost::is_pointer<A6>::value
                      && !::boost::is_pointer<A7>::value
                      && !::boost::is_pointer<A8>::value
                      && !::boost::is_pointer<A9>::value,
                      do_not_pass_raw_pointers_as_bound_argument);
        
        typedef detail::BindState<FunctorType, RunType,
                                  void(typename detail::CallbackParamTraits<A1>::StorageType,
                                       typename detail::CallbackParamTraits<A2>::StorageType,
                                       typename detail::CallbackParamTraits<A3>::StorageType,
                                       typename detail::CallbackParamTraits<A4>::StorageType,
                                       typename detail::CallbackParamTraits<A5>::StorageType,
                                       typename detail::CallbackParamTraits<A6>::StorageType,
                                       typename detail::CallbackParamTraits<A7>::StorageType,
                                       typename detail::CallbackParamTraits<A8>::StorageType,
                                       typename detail::CallbackParamTraits<A9>::StorageType)
                                  > BindState;
        
        return Callback<typename BindState::UnboundRunType>(
                new BindState(FunctorType(functor), a1, a2, a3, a4, a5, a6, a7, a8, a9));
    }

    template <typename T>
    detail::UnretainedWrapper<T> unretained(T *p)
    { return detail::UnretainedWrapper<T>(p); }

    template <typename T>
    detail::OwnedWrapper<T> owned(T *p)
    { return detail::OwnedWrapper<T>(p); }

    template <typename T>
    detail::SharedWrapper<T> shared(const boost::shared_ptr<T>& p)
    { return detail::SharedWrapper<T>(p); }

} // namespace wsd

#endif  // __BIND_H__
