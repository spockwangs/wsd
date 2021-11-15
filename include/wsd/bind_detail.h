#ifndef __BIND_DETAIL_H__
#define __BIND_DETAIL_H__

#include <memory>
#include <type_traits>

#include "callback_detail.h"

namespace wsd {

namespace detail {

template <typename T>
struct is_non_const_reference : std::false_type {
};
template <typename T>
struct is_non_const_reference<T &> : std::true_type {
};
template <typename T>
struct is_non_const_reference<const T &> : std::false_type {
};

// FunctorAdapter<> abstracts away the diff syntaxes of calling functions or class methods.
//
// The following specializes for arities 0 to 9.
template <typename Signature>
class FunctorAdapter;

// Function: arity 0.
template <typename R>
class FunctorAdapter<R (*)()> {
public:
    typedef R(RunType)();

    explicit FunctorAdapter(R (*f)()) : m_pf(f)
    {
    }

    R operator()()
    {
        return (*m_pf)();
    }

private:
    R (*m_pf)();
};

// Method: arity 0.
template <typename R, typename T>
class FunctorAdapter<R (T::*)()> {
public:
    typedef R(RunType)(T *);

    explicit FunctorAdapter(R (T::*pmf)()) : m_pmf(pmf)
    {
    }

    R operator()(T *pobj)
    {
        return (pobj->*m_pmf)();
    }

private:
    R (T::*m_pmf)();
};

// Const method: arity 0.
template <typename R, typename T>
class FunctorAdapter<R (T::*)() const> {
public:
    typedef R(RunType)(const T *);

    explicit FunctorAdapter(R (T::*pmf)() const) : m_pmf(pmf)
    {
    }

    R operator()(const T *pobj)
    {
        return (pobj->*m_pmf)();
    }

private:
    R (T::*m_pmf)() const;
};

// Function: arity 1.
template <typename R, typename A1>
class FunctorAdapter<R (*)(A1)> {
public:
    typedef R(RunType)(A1);

    FunctorAdapter(R (*pf)(A1)) : m_pf(pf)
    {
    }

    R operator()(typename CallbackParamTraits<A1>::ForwardType a1)
    {
        return (*m_pf)(a1);
    }

private:
    R (*m_pf)(A1);
};

// Method: arity 1.
template <typename R, typename T, typename A1>
class FunctorAdapter<R (T::*)(A1)> {
public:
    typedef R(RunType)(T *, A1);

    FunctorAdapter(R (T::*pmf)(A1)) : m_pmf(pmf)
    {
    }

    R operator()(T *pobj, typename CallbackParamTraits<A1>::ForwardType a1)
    {
        return (pobj->*m_pmf)(a1);
    }

private:
    R (T::*m_pmf)(A1);
};

// Const method: arity 1.
template <typename R, typename T, typename A1>
class FunctorAdapter<R (T::*)(A1) const> {
public:
    typedef R(RunType)(const T *, A1);

    FunctorAdapter(R (T::*pmf)(A1) const) : m_pmf(pmf)
    {
    }

    R operator()(const T *pobj, typename CallbackParamTraits<A1>::ForwardType a1)
    {
        return (pobj->*m_pmf)(a1);
    }

private:
    R (T::*m_pmf)(A1) const;
};

// Function: arity 2.
template <typename R, typename A1, typename A2>
class FunctorAdapter<R (*)(A1, A2)> {
public:
    typedef R(RunType)(A1, A2);

    FunctorAdapter(R (*pf)(A1, A2)) : m_pf(pf)
    {
    }

    R operator()(typename CallbackParamTraits<A1>::ForwardType a1, typename CallbackParamTraits<A2>::ForwardType a2)
    {
        return (*m_pf)(a1, a2);
    }

private:
    R (*m_pf)(A1, A2);
};

// Method: arity 2.
template <typename R, typename T, typename A1, typename A2>
class FunctorAdapter<R (T::*)(A1, A2)> {
public:
    typedef R(RunType)(T *, A1, A2);

    FunctorAdapter(R (T::*pmf)(A1, A2)) : m_pmf(pmf)
    {
    }

    R operator()(T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2)
    {
        return (pobj->*m_pmf)(a1, a2);
    }

private:
    R (T::*m_pmf)(A1, A2);
};

// Const method: arity 2.
template <typename R, typename T, typename A1, typename A2>
class FunctorAdapter<R (T::*)(A1, A2) const> {
public:
    typedef R(RunType)(const T *, A1, A2);

    FunctorAdapter(R (T::*pmf)(A1, A2) const) : m_pmf(pmf)
    {
    }

    R operator()(const T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2)
    {
        return (pobj->*m_pmf)(a1, a2);
    }

private:
    R (T::*m_pmf)(A1, A2) const;
};

// Function: arity 3.
template <typename R, typename A1, typename A2, typename A3>
class FunctorAdapter<R (*)(A1, A2, A3)> {
public:
    typedef R(RunType)(A1, A2, A3);

    FunctorAdapter(R (*pf)(A1, A2, A3)) : m_pf(pf)
    {
    }

    R operator()(typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3)
    {
        return (*m_pf)(a1, a2, a3);
    }

private:
    R (*m_pf)(A1, A2, A3);
};

// Method: arity 3.
template <typename R, typename T, typename A1, typename A2, typename A3>
class FunctorAdapter<R (T::*)(A1, A2, A3)> {
public:
    typedef R(RunType)(T *, A1, A2, A3);

    FunctorAdapter(R (T::*pmf)(A1, A2, A3)) : m_pmf(pmf)
    {
    }

    R operator()(T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3)
    {
        return (pobj->*m_pmf)(a1, a2, a3);
    }

private:
    R (T::*m_pmf)(A1, A2, A3);
};

// Const method: arity 3.
template <typename R, typename T, typename A1, typename A2, typename A3>
class FunctorAdapter<R (T::*)(A1, A2, A3) const> {
public:
    typedef R(RunType)(const T *, A1, A2, A3);

    FunctorAdapter(R (T::*pmf)(A1, A2, A3) const) : m_pmf(pmf)
    {
    }

    R operator()(const T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3)
    {
        return (pobj->*m_pmf)(a1, a2, a3);
    }

private:
    R (T::*m_pmf)(A1, A2, A3) const;
};

// Function: arity 4.
template <typename R, typename A1, typename A2, typename A3, typename A4>
class FunctorAdapter<R (*)(A1, A2, A3, A4)> {
public:
    typedef R(RunType)(A1, A2, A3, A4);

    FunctorAdapter(R (*pf)(A1, A2, A3, A4)) : m_pf(pf)
    {
    }

    R operator()(typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4)
    {
        return (*m_pf)(a1, a2, a3, a4);
    }

private:
    R (*m_pf)(A1, A2, A3, A4);
};

// Method: arity 4.
template <typename R, typename T, typename A1, typename A2, typename A3, typename A4>
class FunctorAdapter<R (T::*)(A1, A2, A3, A4)> {
public:
    typedef R(RunType)(T *, A1, A2, A3, A4);

    FunctorAdapter(R (T::*pmf)(A1, A2, A3, A4)) : m_pmf(pmf)
    {
    }

    R operator()(T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4)
    {
        return (pobj->*m_pmf)(a1, a2, a3, a4);
    }

private:
    R (T::*m_pmf)(A1, A2, A3, A4);
};

// Const method: arity 4.
template <typename R, typename T, typename A1, typename A2, typename A3, typename A4>
class FunctorAdapter<R (T::*)(A1, A2, A3, A4) const> {
public:
    typedef R(RunType)(const T *, A1, A2, A3, A4);

    FunctorAdapter(R (T::*pmf)(A1, A2, A3, A4) const) : m_pmf(pmf)
    {
    }

    R operator()(const T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4)
    {
        return (pobj->*m_pmf)(a1, a2, a3, a4);
    }

private:
    R (T::*m_pmf)(A1, A2, A3, A4) const;
};

// Function: arity 5.
template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
class FunctorAdapter<R (*)(A1, A2, A3, A4, A5)> {
public:
    typedef R(RunType)(A1, A2, A3, A4, A5);

    FunctorAdapter(R (*pf)(A1, A2, A3, A4, A5)) : m_pf(pf)
    {
    }

    R operator()(typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4,
                 typename CallbackParamTraits<A5>::ForwardType a5)
    {
        return (*m_pf)(a1, a2, a3, a4, a5);
    }

private:
    R (*m_pf)(A1, A2, A3, A4, A5);
};

// Method: arity 5.
template <typename R, typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
class FunctorAdapter<R (T::*)(A1, A2, A3, A4, A5)> {
public:
    typedef R(RunType)(T *, A1, A2, A3, A4, A5);

    FunctorAdapter(R (T::*pmf)(A1, A2, A3, A4, A5)) : m_pmf(pmf)
    {
    }

    R operator()(T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4,
                 typename CallbackParamTraits<A5>::ForwardType a5)
    {
        return (pobj->*m_pmf)(a1, a2, a3, a4, a5);
    }

private:
    R (T::*m_pmf)(A1, A2, A3, A4, A5);
};

// Const method: arity 5.
template <typename R, typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
class FunctorAdapter<R (T::*)(A1, A2, A3, A4, A5) const> {
public:
    typedef R(RunType)(const T *, A1, A2, A3, A4, A5);

    FunctorAdapter(R (T::*pmf)(A1, A2, A3, A4, A5) const) : m_pmf(pmf)
    {
    }

    R operator()(const T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4,
                 typename CallbackParamTraits<A5>::ForwardType a5)
    {
        return (pobj->*m_pmf)(a1, a2, a3, a4, a5);
    }

private:
    R (T::*m_pmf)(A1, A2, A3, A4, A5) const;
};

// Function: arity 6.
template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
class FunctorAdapter<R (*)(A1, A2, A3, A4, A5, A6)> {
public:
    typedef R(RunType)(A1, A2, A3, A4, A5, A6);

    FunctorAdapter(R (*pf)(A1, A2, A3, A4, A5, A6)) : m_pf(pf)
    {
    }

    R operator()(typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4,
                 typename CallbackParamTraits<A5>::ForwardType a5,
                 typename CallbackParamTraits<A6>::ForwardType a6)
    {
        return (*m_pf)(a1, a2, a3, a4, a5, a6);
    }

private:
    R (*m_pf)(A1, A2, A3, A4, A5, A6);
};

// Method: arity 6.
template <typename R, typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
class FunctorAdapter<R (T::*)(A1, A2, A3, A4, A5, A6)> {
public:
    typedef R(RunType)(T *, A1, A2, A3, A4, A5, A6);

    FunctorAdapter(R (T::*pmf)(A1, A2, A3, A4, A5, A6)) : m_pmf(pmf)
    {
    }

    R operator()(T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4,
                 typename CallbackParamTraits<A5>::ForwardType a5,
                 typename CallbackParamTraits<A6>::ForwardType a6)
    {
        return (pobj->*m_pmf)(a1, a2, a3, a4, a5, a6);
    }

private:
    R (T::*m_pmf)(A1, A2, A3, A4, A5, A6);
};

// Const method: arity 6.
template <typename R, typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
class FunctorAdapter<R (T::*)(A1, A2, A3, A4, A5, A6) const> {
public:
    typedef R(RunType)(const T *, A1, A2, A3, A4, A5, A6);

    FunctorAdapter(R (T::*pmf)(A1, A2, A3, A4, A5, A6) const) : m_pmf(pmf)
    {
    }

    R operator()(const T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4,
                 typename CallbackParamTraits<A5>::ForwardType a5,
                 typename CallbackParamTraits<A6>::ForwardType a6)
    {
        return (pobj->*m_pmf)(a1, a2, a3, a4, a5, a6);
    }

private:
    R (T::*m_pmf)(A1, A2, A3, A4, A5, A6) const;
};

// Function: arity 7.
template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
class FunctorAdapter<R (*)(A1, A2, A3, A4, A5, A6, A7)> {
public:
    typedef R(RunType)(A1, A2, A3, A4, A5, A6, A7);

    FunctorAdapter(R (*pf)(A1, A2, A3, A4, A5, A6, A7)) : m_pf(pf)
    {
    }

    R operator()(typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4,
                 typename CallbackParamTraits<A5>::ForwardType a5,
                 typename CallbackParamTraits<A6>::ForwardType a6,
                 typename CallbackParamTraits<A7>::ForwardType a7)
    {
        return (*m_pf)(a1, a2, a3, a4, a5, a6, a7);
    }

private:
    R (*m_pf)(A1, A2, A3, A4, A5, A6, A7);
};

// Method: arity 7.
template <typename R,
          typename T,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7>
class FunctorAdapter<R (T::*)(A1, A2, A3, A4, A5, A6, A7)> {
public:
    typedef R(RunType)(T *, A1, A2, A3, A4, A5, A6, A7);

    FunctorAdapter(R (T::*pmf)(A1, A2, A3, A4, A5, A6, A7)) : m_pmf(pmf)
    {
    }

    R operator()(T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4,
                 typename CallbackParamTraits<A5>::ForwardType a5,
                 typename CallbackParamTraits<A6>::ForwardType a6,
                 typename CallbackParamTraits<A7>::ForwardType a7)
    {
        return (pobj->*m_pmf)(a1, a2, a3, a4, a5, a6, a7);
    }

private:
    R (T::*m_pmf)(A1, A2, A3, A4, A5, A6, A7);
};

// Const method: arity 7.
template <typename R,
          typename T,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7>
class FunctorAdapter<R (T::*)(A1, A2, A3, A4, A5, A6, A7) const> {
public:
    typedef R(RunType)(const T *, A1, A2, A3, A4, A5, A6, A7);

    FunctorAdapter(R (T::*pmf)(A1, A2, A3, A4, A5, A6, A7) const) : m_pmf(pmf)
    {
    }

    R operator()(const T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4,
                 typename CallbackParamTraits<A5>::ForwardType a5,
                 typename CallbackParamTraits<A6>::ForwardType a6,
                 typename CallbackParamTraits<A7>::ForwardType a7)
    {
        return (pobj->*m_pmf)(a1, a2, a3, a4, a5, a6, a7);
    }

private:
    R (T::*m_pmf)(A1, A2, A3, A4, A5, A6, A7) const;
};

// Function: arity 8.
template <typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8>
class FunctorAdapter<R (*)(A1, A2, A3, A4, A5, A6, A7, A8)> {
public:
    typedef R(RunType)(A1, A2, A3, A4, A5, A6, A7, A8);

    FunctorAdapter(R (*pf)(A1, A2, A3, A4, A5, A6, A7, A8)) : m_pf(pf)
    {
    }

    R operator()(typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4,
                 typename CallbackParamTraits<A5>::ForwardType a5,
                 typename CallbackParamTraits<A6>::ForwardType a6,
                 typename CallbackParamTraits<A7>::ForwardType a7,
                 typename CallbackParamTraits<A8>::ForwardType a8)
    {
        return (*m_pf)(a1, a2, a3, a4, a5, a6, a7, a8);
    }

private:
    R (*m_pf)(A1, A2, A3, A4, A5, A6, A7, A8);
};

// Method: arity 8.
template <typename R,
          typename T,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8>
class FunctorAdapter<R (T::*)(A1, A2, A3, A4, A5, A6, A7, A8)> {
public:
    typedef R(RunType)(T *, A1, A2, A3, A4, A5, A6, A7, A8);

    FunctorAdapter(R (T::*pmf)(A1, A2, A3, A4, A5, A6, A7, A8)) : m_pmf(pmf)
    {
    }

    R operator()(T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4,
                 typename CallbackParamTraits<A5>::ForwardType a5,
                 typename CallbackParamTraits<A6>::ForwardType a6,
                 typename CallbackParamTraits<A7>::ForwardType a7,
                 typename CallbackParamTraits<A8>::ForwardType a8)
    {
        return (pobj->*m_pmf)(a1, a2, a3, a4, a5, a6, a7, a8);
    }

private:
    R (T::*m_pmf)(A1, A2, A3, A4, A5, A6, A7, A8);
};

// Const method: arity 8.
template <typename R,
          typename T,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8>
class FunctorAdapter<R (T::*)(A1, A2, A3, A4, A5, A6, A7, A8) const> {
public:
    typedef R(RunType)(const T *, A1, A2, A3, A4, A5, A6, A7, A8);

    FunctorAdapter(R (T::*pmf)(A1, A2, A3, A4, A5, A6, A7, A8) const) : m_pmf(pmf)
    {
    }

    R operator()(const T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4,
                 typename CallbackParamTraits<A5>::ForwardType a5,
                 typename CallbackParamTraits<A6>::ForwardType a6,
                 typename CallbackParamTraits<A7>::ForwardType a7,
                 typename CallbackParamTraits<A8>::ForwardType a8)
    {
        return (pobj->*m_pmf)(a1, a2, a3, a4, a5, a6, a7, a8);
    }

private:
    R (T::*m_pmf)(A1, A2, A3, A4, A5, A6, A7, A8) const;
};

// Function: arity 9.
template <typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
class FunctorAdapter<R (*)(A1, A2, A3, A4, A5, A6, A7, A8, A9)> {
public:
    typedef R(RunType)(A1, A2, A3, A4, A5, A6, A7, A8, A9);

    FunctorAdapter(R (*pf)(A1, A2, A3, A4, A5, A6, A7, A8, A9)) : m_pf(pf)
    {
    }

    R operator()(typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4,
                 typename CallbackParamTraits<A5>::ForwardType a5,
                 typename CallbackParamTraits<A6>::ForwardType a6,
                 typename CallbackParamTraits<A7>::ForwardType a7,
                 typename CallbackParamTraits<A8>::ForwardType a8,
                 typename CallbackParamTraits<A9>::ForwardType a9)
    {
        return (*m_pf)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }

private:
    R (*m_pf)(A1, A2, A3, A4, A5, A6, A7, A8, A9);
};

// Method: arity 9.
template <typename R,
          typename T,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
class FunctorAdapter<R (T::*)(A1, A2, A3, A4, A5, A6, A7, A8, A9)> {
public:
    typedef R(RunType)(T *, A1, A2, A3, A4, A5, A6, A7, A8, A9);

    FunctorAdapter(R (T::*pmf)(A1, A2, A3, A4, A5, A6, A7, A8, A9)) : m_pmf(pmf)
    {
    }

    R operator()(T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4,
                 typename CallbackParamTraits<A5>::ForwardType a5,
                 typename CallbackParamTraits<A6>::ForwardType a6,
                 typename CallbackParamTraits<A7>::ForwardType a7,
                 typename CallbackParamTraits<A8>::ForwardType a8,
                 typename CallbackParamTraits<A9>::ForwardType a9)
    {
        return (pobj->*m_pmf)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }

private:
    R (T::*m_pmf)(A1, A2, A3, A4, A5, A6, A7, A8, A9);
};

// Const method: arity 9.
template <typename R,
          typename T,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
class FunctorAdapter<R (T::*)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const> {
public:
    typedef R(RunType)(const T *, A1, A2, A3, A4, A5, A6, A7, A8, A9);

    FunctorAdapter(R (T::*pmf)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const) : m_pmf(pmf)
    {
    }

    R operator()(const T *pobj,
                 typename CallbackParamTraits<A1>::ForwardType a1,
                 typename CallbackParamTraits<A2>::ForwardType a2,
                 typename CallbackParamTraits<A3>::ForwardType a3,
                 typename CallbackParamTraits<A4>::ForwardType a4,
                 typename CallbackParamTraits<A5>::ForwardType a5,
                 typename CallbackParamTraits<A6>::ForwardType a6,
                 typename CallbackParamTraits<A7>::ForwardType a7,
                 typename CallbackParamTraits<A8>::ForwardType a8,
                 typename CallbackParamTraits<A9>::ForwardType a9)
    {
        return (pobj->*m_pmf)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }

private:
    R (T::*m_pmf)(A1, A2, A3, A4, A5, A6, A7, A8, A9) const;
};

// Argument wrappers can be used to specify memory management semantics of
// arguments that are bound by bind().
//
// UnretainedWrapper specifies the caller be responsible for managing the memory
// of the bound parameters.
template <typename T>
class UnretainedWrapper {
public:
    explicit UnretainedWrapper(T *p) : m_ptr(p)
    {
    }
    T *get() const
    {
        return m_ptr;
    }

private:
    T *m_ptr;
};

// OwnedWrapper specifies the callback be responsible for managing the memory of
// the bound parameters, that is, the pointee is destroyed when callback is destroyed.
template <typename T>
class OwnedWrapper {
public:
    explicit OwnedWrapper(T *p) : m_ptr(p)
    {
    }
    ~OwnedWrapper()
    {
        delete m_ptr;
    }
    OwnedWrapper(const OwnedWrapper &o)
    {
        m_ptr = o.m_ptr;
        o.m_ptr = NULL;
    }
    OwnedWrapper &operator=(const OwnedWrapper &o)
    {
        delete m_ptr;
        m_ptr = o.m_ptr;
        o.m_ptr = NULL;
    }
    T *get() const
    {
        return m_ptr;
    }

private:
    mutable T *m_ptr;
};

// SharedWrapper sepcifies the object is shared by the caller and the callback. It
// is deleted when the last reference is gone.
template <typename T>
struct SharedWrapper {
public:
    explicit SharedWrapper(const std::shared_ptr<T> &p) : m_ptr(p)
    {
    }
    T *get() const
    {
        return m_ptr.get();
    }

private:
    std::shared_ptr<T> m_ptr;
};

// Unwrap the stored parameters for the wrappers above.
template <typename T>
struct UnwrapTraits {
    typedef const T &ForwardType;
    static ForwardType unwrap(const T &o)
    {
        return o;
    }
};

template <typename T>
struct UnwrapTraits<UnretainedWrapper<T>> {
    typedef T *ForwardType;
    static ForwardType unwrap(const UnretainedWrapper<T> &unretained)
    {
        return unretained.get();
    }
};

template <typename T>
struct UnwrapTraits<OwnedWrapper<T>> {
    typedef T *ForwardType;
    static ForwardType unwrap(const OwnedWrapper<T> &owned)
    {
        return owned.get();
    }
};

template <typename T>
struct UnwrapTraits<SharedWrapper<T>> {
    typedef T *ForwardType;
    static ForwardType unwrap(const SharedWrapper<T> &shared)
    {
        return shared.get();
    }
};

// Invoker<>
//
// Unwraps the curried parameters and executes the callback.
// There are (ARITY^2 + ARITY)/2 invoker types.
template <typename BindState, typename RunType, int NumBound>
class Invoker;

// Arity: 0 -> 0
template <typename BindState, typename R>
class Invoker<BindState, R(), 0> {
public:
    typedef R(UnboundRunType)();

    static R invoke(BindStateBase *base)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        return (bind_state->m_functor)();
    }
};

// Arity: 1 -> 1
template <typename BindState, typename R, typename A1>
class Invoker<BindState, R(A1), 0> {
public:
    typedef R(UnboundRunType)(A1);

    static R invoke(BindStateBase *base, typename CallbackParamTraits<A1>::ForwardType a1)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        return (bind_state->m_functor)(a1);
    }
};

// Arity: 1 -> 0
template <typename BindState, typename R, typename A1>
class Invoker<BindState, R(A1), 1> {
public:
    typedef R(UnboundRunType)();

    static R invoke(BindStateBase *base)
    {
        BindState *bind_state = static_cast<BindState *>(base);

        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        return (bind_state->m_functor)(a1);
    }
};

// Arity: 2 -> 2
template <typename BindState, typename R, typename A1, typename A2>
class Invoker<BindState, R(A1, A2), 0> {
public:
    typedef R(UnboundRunType)(A1, A2);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A1>::ForwardType a1,
                    typename CallbackParamTraits<A2>::ForwardType a2)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        return (bind_state->m_functor)(a1, a2);
    }
};

// Arity: 2 -> 1
template <typename BindState, typename R, typename A1, typename A2>
class Invoker<BindState, R(A1, A2), 1> {
public:
    typedef R(UnboundRunType)(A2);

    static R invoke(BindStateBase *base, typename CallbackParamTraits<A2>::ForwardType a2)
    {
        BindState *bind_state = static_cast<BindState *>(base);

        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        return (bind_state->m_functor)(a1, a2);
    }
};

// Arity: 2 -> 0
template <typename BindState, typename R, typename A1, typename A2>
class Invoker<BindState, R(A1, A2), 2> {
public:
    typedef R(UnboundRunType)();

    static R invoke(BindStateBase *base)
    {
        BindState *bind_state = static_cast<BindState *>(base);

        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        return (bind_state->m_functor)(a1, a2);
    }
};

// Arity: 3 -> 3
template <typename BindState, typename R, typename A1, typename A2, typename A3>
class Invoker<BindState, R(A1, A2, A3), 0> {
public:
    typedef R(UnboundRunType)(A1, A2, A3);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A1>::ForwardType a1,
                    typename CallbackParamTraits<A2>::ForwardType a2,
                    typename CallbackParamTraits<A3>::ForwardType a3)
    {
        BindState *bind_state = static_cast<BindState *>(base);

        return (bind_state->m_functor)(a1, a2, a3);
    }
};

// Arity: 3 -> 2
template <typename BindState, typename R, typename A1, typename A2, typename A3>
class Invoker<BindState, R(A1, A2, A3), 1> {
public:
    typedef R(UnboundRunType)(A2, A3);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A2>::ForwardType a2,
                    typename CallbackParamTraits<A3>::ForwardType a3)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        return (bind_state->m_functor)(a1, a2, a3);
    }
};

// Arity: 3 -> 1
template <typename BindState, typename R, typename A1, typename A2, typename A3>
class Invoker<BindState, R(A1, A2, A3), 2> {
public:
    typedef R(UnboundRunType)(A3);

    static R invoke(BindStateBase *base, typename CallbackParamTraits<A3>::ForwardType a3)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        return (bind_state->m_functor)(a1, a2, a3);
    }
};

// Arity: 3 -> 0
template <typename BindState, typename R, typename A1, typename A2, typename A3>
class Invoker<BindState, R(A1, A2, A3), 3> {
public:
    typedef R(UnboundRunType)();

    static R invoke(BindStateBase *base)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        return (bind_state->m_functor)(a1, a2, a3);
    }
};

// Arity: 4 -> 4
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4>
class Invoker<BindState, R(A1, A2, A3, A4), 0> {
public:
    typedef R(UnboundRunType)(A1, A2, A3, A4);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A1>::ForwardType a1,
                    typename CallbackParamTraits<A2>::ForwardType a2,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4)
    {
        BindState *bind_state = static_cast<BindState *>(base);

        return (bind_state->m_functor)(a1, a2, a3, a4);
    }
};

// Arity: 4 -> 3
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4>
class Invoker<BindState, R(A1, A2, A3, A4), 1> {
public:
    typedef R(UnboundRunType)(A2, A3, A4);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A2>::ForwardType a2,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        return (bind_state->m_functor)(a1, a2, a3, a4);
    }
};

// Arity: 4 -> 2
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4>
class Invoker<BindState, R(A1, A2, A3, A4), 2> {
public:
    typedef R(UnboundRunType)(A3, A4);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        return (bind_state->m_functor)(a1, a2, a3, a4);
    }
};

// Arity: 4 -> 1
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4>
class Invoker<BindState, R(A1, A2, A3, A4), 3> {
public:
    typedef R(UnboundRunType)(A4);

    static R invoke(BindStateBase *base, typename CallbackParamTraits<A4>::ForwardType a4)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        return (bind_state->m_functor)(a1, a2, a3, a4);
    }
};

// Arity: 4 -> 0
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4>
class Invoker<BindState, R(A1, A2, A3, A4), 4> {
public:
    typedef R(UnboundRunType)();

    static R invoke(BindStateBase *base)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        return (bind_state->m_functor)(a1, a2, a3, a4);
    }
};

// Arity: 5 -> 5
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
class Invoker<BindState, R(A1, A2, A3, A4, A5), 0> {
public:
    typedef R(UnboundRunType)(A1, A2, A3, A4, A5);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A1>::ForwardType a1,
                    typename CallbackParamTraits<A2>::ForwardType a2,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5)
    {
        BindState *bind_state = static_cast<BindState *>(base);

        return (bind_state->m_functor)(a1, a2, a3, a4, a5);
    }
};

// Arity: 5 -> 4
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
class Invoker<BindState, R(A1, A2, A3, A4, A5), 1> {
public:
    typedef R(UnboundRunType)(A2, A3, A4, A5);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A2>::ForwardType a2,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5);
    }
};

// Arity: 5 -> 3
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
class Invoker<BindState, R(A1, A2, A3, A4, A5), 2> {
public:
    typedef R(UnboundRunType)(A3, A4, A5);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5);
    }
};

// Arity: 5 -> 2
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
class Invoker<BindState, R(A1, A2, A3, A4, A5), 3> {
public:
    typedef R(UnboundRunType)(A4, A5);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5);
    }
};

// Arity: 5 -> 1
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
class Invoker<BindState, R(A1, A2, A3, A4, A5), 4> {
public:
    typedef R(UnboundRunType)(A5);

    static R invoke(BindStateBase *base, typename CallbackParamTraits<A5>::ForwardType a5)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5);
    }
};

// Arity: 5 -> 0
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
class Invoker<BindState, R(A1, A2, A3, A4, A5), 5> {
public:
    typedef R(UnboundRunType)();

    static R invoke(BindStateBase *base)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        typename Bound5UnwrapTraits::ForwardType a5 = Bound5UnwrapTraits::unwrap(bind_state->m_a5);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5);
    }
};

// Arity: 6 -> 6
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6), 0> {
public:
    typedef R(UnboundRunType)(A1, A2, A3, A4, A5, A6);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A1>::ForwardType a1,
                    typename CallbackParamTraits<A2>::ForwardType a2,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6)
    {
        BindState *bind_state = static_cast<BindState *>(base);

        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6);
    }
};

// Arity: 6 -> 5
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6), 1> {
public:
    typedef R(UnboundRunType)(A2, A3, A4, A5, A6);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A2>::ForwardType a2,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6);
    }
};

// Arity: 6 -> 4
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6), 2> {
public:
    typedef R(UnboundRunType)(A3, A4, A5, A6);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6);
    }
};

// Arity: 6 -> 3
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6), 3> {
public:
    typedef R(UnboundRunType)(A4, A5, A6);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6);
    }
};

// Arity: 6 -> 2
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6), 4> {
public:
    typedef R(UnboundRunType)(A5, A6);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6);
    }
};

// Arity: 6 -> 1
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6), 5> {
public:
    typedef R(UnboundRunType)(A6);

    static R invoke(BindStateBase *base, typename CallbackParamTraits<A6>::ForwardType a6)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        typename Bound5UnwrapTraits::ForwardType a5 = Bound5UnwrapTraits::unwrap(bind_state->m_a5);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6);
    }
};

// Arity: 6 -> 0
template <typename BindState, typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6), 6> {
public:
    typedef R(UnboundRunType)();

    static R invoke(BindStateBase *base)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typedef typename BindState::Bound6UnwrapTraits Bound6UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        typename Bound5UnwrapTraits::ForwardType a5 = Bound5UnwrapTraits::unwrap(bind_state->m_a5);
        typename Bound6UnwrapTraits::ForwardType a6 = Bound6UnwrapTraits::unwrap(bind_state->m_a6);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6);
    }
};

// Arity: 7 -> 7
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7), 0> {
public:
    typedef R(UnboundRunType)(A1, A2, A3, A4, A5, A6, A7);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A1>::ForwardType a1,
                    typename CallbackParamTraits<A2>::ForwardType a2,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7)
    {
        BindState *bind_state = static_cast<BindState *>(base);

        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7);
    }
};

// Arity: 7 -> 6
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7), 1> {
public:
    typedef R(UnboundRunType)(A2, A3, A4, A5, A6, A7);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A2>::ForwardType a2,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7);
    }
};

// Arity: 7 -> 5
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7), 2> {
public:
    typedef R(UnboundRunType)(A3, A4, A5, A6, A7);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7);
    }
};

// Arity: 7 -> 4
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7), 3> {
public:
    typedef R(UnboundRunType)(A4, A5, A6, A7);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7);
    }
};

// Arity: 7 -> 3
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7), 4> {
public:
    typedef R(UnboundRunType)(A5, A6, A7);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7);
    }
};

// Arity: 7 -> 2
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7), 5> {
public:
    typedef R(UnboundRunType)(A6, A7);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        typename Bound5UnwrapTraits::ForwardType a5 = Bound5UnwrapTraits::unwrap(bind_state->m_a5);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7);
    }
};

// Arity: 7 -> 1
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7), 6> {
public:
    typedef R(UnboundRunType)(A7);

    static R invoke(BindStateBase *base, typename CallbackParamTraits<A7>::ForwardType a7)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typedef typename BindState::Bound6UnwrapTraits Bound6UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        typename Bound5UnwrapTraits::ForwardType a5 = Bound5UnwrapTraits::unwrap(bind_state->m_a5);
        typename Bound6UnwrapTraits::ForwardType a6 = Bound6UnwrapTraits::unwrap(bind_state->m_a6);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7);
    }
};

// Arity: 7 -> 0
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7), 7> {
public:
    typedef R(UnboundRunType)();

    static R invoke(BindStateBase *base)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typedef typename BindState::Bound6UnwrapTraits Bound6UnwrapTraits;
        typedef typename BindState::Bound7UnwrapTraits Bound7UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        typename Bound5UnwrapTraits::ForwardType a5 = Bound5UnwrapTraits::unwrap(bind_state->m_a5);
        typename Bound6UnwrapTraits::ForwardType a6 = Bound6UnwrapTraits::unwrap(bind_state->m_a6);
        typename Bound7UnwrapTraits::ForwardType a7 = Bound7UnwrapTraits::unwrap(bind_state->m_a7);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7);
    }
};

// Arity: 8 -> 8
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8), 0> {
public:
    typedef R(UnboundRunType)(A1, A2, A3, A4, A5, A6, A7, A8);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A1>::ForwardType a1,
                    typename CallbackParamTraits<A2>::ForwardType a2,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7,
                    typename CallbackParamTraits<A8>::ForwardType a8)
    {
        BindState *bind_state = static_cast<BindState *>(base);

        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8);
    }
};

// Arity: 8 -> 7
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8), 1> {
public:
    typedef R(UnboundRunType)(A2, A3, A4, A5, A6, A7, A8);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A2>::ForwardType a2,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7,
                    typename CallbackParamTraits<A8>::ForwardType a8)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8);
    }
};

// Arity: 8 -> 6
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8), 2> {
public:
    typedef R(UnboundRunType)(A3, A4, A5, A6, A7, A8);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7,
                    typename CallbackParamTraits<A8>::ForwardType a8)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8);
    }
};

// Arity: 8 -> 5
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8), 3> {
public:
    typedef R(UnboundRunType)(A4, A5, A6, A7, A8);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7,
                    typename CallbackParamTraits<A8>::ForwardType a8)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8);
    }
};

// Arity: 8 -> 4
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8), 4> {
public:
    typedef R(UnboundRunType)(A5, A6, A7, A8);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7,
                    typename CallbackParamTraits<A8>::ForwardType a8)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8);
    }
};

// Arity: 8 -> 3
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8), 5> {
public:
    typedef R(UnboundRunType)(A6, A7, A8);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7,
                    typename CallbackParamTraits<A8>::ForwardType a8)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        typename Bound5UnwrapTraits::ForwardType a5 = Bound5UnwrapTraits::unwrap(bind_state->m_a5);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8);
    }
};

// Arity: 8 -> 2
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8), 6> {
public:
    typedef R(UnboundRunType)(A7, A8);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A7>::ForwardType a7,
                    typename CallbackParamTraits<A8>::ForwardType a8)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typedef typename BindState::Bound6UnwrapTraits Bound6UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        typename Bound5UnwrapTraits::ForwardType a5 = Bound5UnwrapTraits::unwrap(bind_state->m_a5);
        typename Bound6UnwrapTraits::ForwardType a6 = Bound6UnwrapTraits::unwrap(bind_state->m_a6);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8);
    }
};

// Arity: 8 -> 1
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8), 7> {
public:
    typedef R(UnboundRunType)(A8);

    static R invoke(BindStateBase *base, typename CallbackParamTraits<A8>::ForwardType a8)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typedef typename BindState::Bound6UnwrapTraits Bound6UnwrapTraits;
        typedef typename BindState::Bound7UnwrapTraits Bound7UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        typename Bound5UnwrapTraits::ForwardType a5 = Bound5UnwrapTraits::unwrap(bind_state->m_a5);
        typename Bound6UnwrapTraits::ForwardType a6 = Bound6UnwrapTraits::unwrap(bind_state->m_a6);
        typename Bound7UnwrapTraits::ForwardType a7 = Bound7UnwrapTraits::unwrap(bind_state->m_a7);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8);
    }
};

// Arity: 8 -> 0
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8), 8> {
public:
    typedef R(UnboundRunType)();

    static R invoke(BindStateBase *base)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typedef typename BindState::Bound6UnwrapTraits Bound6UnwrapTraits;
        typedef typename BindState::Bound7UnwrapTraits Bound7UnwrapTraits;
        typedef typename BindState::Bound8UnwrapTraits Bound8UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        typename Bound5UnwrapTraits::ForwardType a5 = Bound5UnwrapTraits::unwrap(bind_state->m_a5);
        typename Bound6UnwrapTraits::ForwardType a6 = Bound6UnwrapTraits::unwrap(bind_state->m_a6);
        typename Bound7UnwrapTraits::ForwardType a7 = Bound7UnwrapTraits::unwrap(bind_state->m_a7);
        typename Bound8UnwrapTraits::ForwardType a8 = Bound8UnwrapTraits::unwrap(bind_state->m_a8);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8);
    }
};

// Arity: 9 -> 9
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8, A9), 0> {
public:
    typedef R(UnboundRunType)(A1, A2, A3, A4, A5, A6, A7, A8, A9);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A1>::ForwardType a1,
                    typename CallbackParamTraits<A2>::ForwardType a2,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7,
                    typename CallbackParamTraits<A8>::ForwardType a8,
                    typename CallbackParamTraits<A9>::ForwardType a9)
    {
        BindState *bind_state = static_cast<BindState *>(base);

        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
};

// Arity: 9 -> 8
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8, A9), 1> {
public:
    typedef R(UnboundRunType)(A2, A3, A4, A5, A6, A7, A8, A9);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A2>::ForwardType a2,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7,
                    typename CallbackParamTraits<A8>::ForwardType a8,
                    typename CallbackParamTraits<A9>::ForwardType a9)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
};

// Arity: 9 -> 7
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8, A9), 2> {
public:
    typedef R(UnboundRunType)(A3, A4, A5, A6, A7, A8, A9);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A3>::ForwardType a3,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7,
                    typename CallbackParamTraits<A8>::ForwardType a8,
                    typename CallbackParamTraits<A9>::ForwardType a9)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
};

// Arity: 9 -> 6
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8, A9), 3> {
public:
    typedef R(UnboundRunType)(A4, A5, A6, A7, A8, A9);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A4>::ForwardType a4,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7,
                    typename CallbackParamTraits<A8>::ForwardType a8,
                    typename CallbackParamTraits<A9>::ForwardType a9)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
};

// Arity: 9 -> 5
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8, A9), 4> {
public:
    typedef R(UnboundRunType)(A5, A6, A7, A8, A9);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A5>::ForwardType a5,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7,
                    typename CallbackParamTraits<A8>::ForwardType a8,
                    typename CallbackParamTraits<A9>::ForwardType a9)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
};

// Arity: 9 -> 4
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8, A9), 5> {
public:
    typedef R(UnboundRunType)(A6, A7, A8, A9);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A6>::ForwardType a6,
                    typename CallbackParamTraits<A7>::ForwardType a7,
                    typename CallbackParamTraits<A8>::ForwardType a8,
                    typename CallbackParamTraits<A9>::ForwardType a9)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        typename Bound5UnwrapTraits::ForwardType a5 = Bound5UnwrapTraits::unwrap(bind_state->m_a5);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
};

// Arity: 9 -> 3
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8, A9), 6> {
public:
    typedef R(UnboundRunType)(A7, A8, A9);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A7>::ForwardType a7,
                    typename CallbackParamTraits<A8>::ForwardType a8,
                    typename CallbackParamTraits<A9>::ForwardType a9)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typedef typename BindState::Bound6UnwrapTraits Bound6UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        typename Bound5UnwrapTraits::ForwardType a5 = Bound5UnwrapTraits::unwrap(bind_state->m_a5);
        typename Bound6UnwrapTraits::ForwardType a6 = Bound6UnwrapTraits::unwrap(bind_state->m_a6);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
};

// Arity: 9 -> 2
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8, A9), 7> {
public:
    typedef R(UnboundRunType)(A8, A9);

    static R invoke(BindStateBase *base,
                    typename CallbackParamTraits<A8>::ForwardType a8,
                    typename CallbackParamTraits<A9>::ForwardType a9)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typedef typename BindState::Bound6UnwrapTraits Bound6UnwrapTraits;
        typedef typename BindState::Bound7UnwrapTraits Bound7UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        typename Bound5UnwrapTraits::ForwardType a5 = Bound5UnwrapTraits::unwrap(bind_state->m_a5);
        typename Bound6UnwrapTraits::ForwardType a6 = Bound6UnwrapTraits::unwrap(bind_state->m_a6);
        typename Bound7UnwrapTraits::ForwardType a7 = Bound7UnwrapTraits::unwrap(bind_state->m_a7);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
};

// Arity: 9 -> 1
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8, A9), 8> {
public:
    typedef R(UnboundRunType)(A9);

    static R invoke(BindStateBase *base, typename CallbackParamTraits<A9>::ForwardType a9)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typedef typename BindState::Bound6UnwrapTraits Bound6UnwrapTraits;
        typedef typename BindState::Bound7UnwrapTraits Bound7UnwrapTraits;
        typedef typename BindState::Bound8UnwrapTraits Bound8UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        typename Bound5UnwrapTraits::ForwardType a5 = Bound5UnwrapTraits::unwrap(bind_state->m_a5);
        typename Bound6UnwrapTraits::ForwardType a6 = Bound6UnwrapTraits::unwrap(bind_state->m_a6);
        typename Bound7UnwrapTraits::ForwardType a7 = Bound7UnwrapTraits::unwrap(bind_state->m_a7);
        typename Bound8UnwrapTraits::ForwardType a8 = Bound8UnwrapTraits::unwrap(bind_state->m_a8);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
};

// Arity: 9 -> 0
template <typename BindState,
          typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
class Invoker<BindState, R(A1, A2, A3, A4, A5, A6, A7, A8, A9), 9> {
public:
    typedef R(UnboundRunType)();

    static R invoke(BindStateBase *base)
    {
        BindState *bind_state = static_cast<BindState *>(base);
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typedef typename BindState::Bound6UnwrapTraits Bound6UnwrapTraits;
        typedef typename BindState::Bound7UnwrapTraits Bound7UnwrapTraits;
        typedef typename BindState::Bound8UnwrapTraits Bound8UnwrapTraits;
        typedef typename BindState::Bound9UnwrapTraits Bound9UnwrapTraits;
        typename Bound1UnwrapTraits::ForwardType a1 = Bound1UnwrapTraits::unwrap(bind_state->m_a1);
        typename Bound2UnwrapTraits::ForwardType a2 = Bound2UnwrapTraits::unwrap(bind_state->m_a2);
        typename Bound3UnwrapTraits::ForwardType a3 = Bound3UnwrapTraits::unwrap(bind_state->m_a3);
        typename Bound4UnwrapTraits::ForwardType a4 = Bound4UnwrapTraits::unwrap(bind_state->m_a4);
        typename Bound5UnwrapTraits::ForwardType a5 = Bound5UnwrapTraits::unwrap(bind_state->m_a5);
        typename Bound6UnwrapTraits::ForwardType a6 = Bound6UnwrapTraits::unwrap(bind_state->m_a6);
        typename Bound7UnwrapTraits::ForwardType a7 = Bound7UnwrapTraits::unwrap(bind_state->m_a7);
        typename Bound8UnwrapTraits::ForwardType a8 = Bound8UnwrapTraits::unwrap(bind_state->m_a8);
        typename Bound9UnwrapTraits::ForwardType a9 = Bound9UnwrapTraits::unwrap(bind_state->m_a9);
        return (bind_state->m_functor)(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
};

// BindState<>
//
// This stores all the curreid parameters passed to bind(). There are ARITY
// BindState types.
template <typename FunctorType, typename RunType, typename BoundArgsType>
class BindState;

template <typename FunctorType, typename RunType>
class BindState<FunctorType, RunType, void()> : public BindStateBase {
public:
    typedef Invoker<BindState, RunType, 0> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;

    explicit BindState(const FunctorType &functor) : m_functor(functor)
    {
    }

    FunctorType m_functor;
};

template <typename FunctorType, typename RunType, typename A1>
class BindState<FunctorType, RunType, void(A1)> : public BindStateBase {
public:
    typedef Invoker<BindState, RunType, 1> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;
    typedef UnwrapTraits<A1> Bound1UnwrapTraits;

    BindState(const FunctorType &functor, const A1 &a1) : m_functor(functor), m_a1(a1)
    {
    }

    FunctorType m_functor;
    A1 m_a1;
};

template <typename FunctorType, typename RunType, typename A1, typename A2>
class BindState<FunctorType, RunType, void(A1, A2)> : public BindStateBase {
public:
    typedef Invoker<BindState, RunType, 2> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;
    typedef UnwrapTraits<A1> Bound1UnwrapTraits;
    typedef UnwrapTraits<A2> Bound2UnwrapTraits;

    BindState(const FunctorType &functor, const A1 &a1, const A2 &a2) : m_functor(functor), m_a1(a1), m_a2(a2)
    {
    }

    FunctorType m_functor;
    A1 m_a1;
    A2 m_a2;
};

template <typename FunctorType, typename RunType, typename A1, typename A2, typename A3>
class BindState<FunctorType, RunType, void(A1, A2, A3)> : public BindStateBase {
public:
    typedef Invoker<BindState, RunType, 3> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;
    typedef UnwrapTraits<A1> Bound1UnwrapTraits;
    typedef UnwrapTraits<A2> Bound2UnwrapTraits;
    typedef UnwrapTraits<A3> Bound3UnwrapTraits;

    BindState(const FunctorType &functor, const A1 &a1, const A2 &a2, const A3 &a3)
        : m_functor(functor), m_a1(a1), m_a2(a2), m_a3(a3)
    {
    }

    FunctorType m_functor;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
};

template <typename FunctorType, typename RunType, typename A1, typename A2, typename A3, typename A4>
class BindState<FunctorType, RunType, void(A1, A2, A3, A4)> : public BindStateBase {
public:
    typedef Invoker<BindState, RunType, 4> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;
    typedef UnwrapTraits<A1> Bound1UnwrapTraits;
    typedef UnwrapTraits<A2> Bound2UnwrapTraits;
    typedef UnwrapTraits<A3> Bound3UnwrapTraits;
    typedef UnwrapTraits<A4> Bound4UnwrapTraits;

    BindState(const FunctorType &functor, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
        : m_functor(functor), m_a1(a1), m_a2(a2), m_a3(a3), m_a4(a4)
    {
    }

    FunctorType m_functor;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
    A4 m_a4;
};

template <typename FunctorType, typename RunType, typename A1, typename A2, typename A3, typename A4, typename A5>
class BindState<FunctorType, RunType, void(A1, A2, A3, A4, A5)> : public BindStateBase {
public:
    typedef Invoker<BindState, RunType, 5> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;
    typedef UnwrapTraits<A1> Bound1UnwrapTraits;
    typedef UnwrapTraits<A2> Bound2UnwrapTraits;
    typedef UnwrapTraits<A3> Bound3UnwrapTraits;
    typedef UnwrapTraits<A4> Bound4UnwrapTraits;
    typedef UnwrapTraits<A5> Bound5UnwrapTraits;

    BindState(const FunctorType &functor, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
        : m_functor(functor), m_a1(a1), m_a2(a2), m_a3(a3), m_a4(a4), m_a5(a5)
    {
    }

    FunctorType m_functor;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
    A4 m_a4;
    A5 m_a5;
};

template <typename FunctorType,
          typename RunType,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6>
class BindState<FunctorType, RunType, void(A1, A2, A3, A4, A5, A6)> : public BindStateBase {
public:
    typedef Invoker<BindState, RunType, 6> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;
    typedef UnwrapTraits<A1> Bound1UnwrapTraits;
    typedef UnwrapTraits<A2> Bound2UnwrapTraits;
    typedef UnwrapTraits<A3> Bound3UnwrapTraits;
    typedef UnwrapTraits<A4> Bound4UnwrapTraits;
    typedef UnwrapTraits<A5> Bound5UnwrapTraits;
    typedef UnwrapTraits<A6> Bound6UnwrapTraits;

    BindState(const FunctorType &functor,
              const A1 &a1,
              const A2 &a2,
              const A3 &a3,
              const A4 &a4,
              const A5 &a5,
              const A6 &a6)
        : m_functor(functor), m_a1(a1), m_a2(a2), m_a3(a3), m_a4(a4), m_a5(a5), m_a6(a6)
    {
    }

    FunctorType m_functor;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
    A4 m_a4;
    A5 m_a5;
    A6 m_a6;
};

template <typename FunctorType,
          typename RunType,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7>
class BindState<FunctorType, RunType, void(A1, A2, A3, A4, A5, A6, A7)> : public BindStateBase {
public:
    typedef Invoker<BindState, RunType, 7> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;
    typedef UnwrapTraits<A1> Bound1UnwrapTraits;
    typedef UnwrapTraits<A2> Bound2UnwrapTraits;
    typedef UnwrapTraits<A3> Bound3UnwrapTraits;
    typedef UnwrapTraits<A4> Bound4UnwrapTraits;
    typedef UnwrapTraits<A5> Bound5UnwrapTraits;
    typedef UnwrapTraits<A6> Bound6UnwrapTraits;
    typedef UnwrapTraits<A7> Bound7UnwrapTraits;

    BindState(const FunctorType &functor,
              const A1 &a1,
              const A2 &a2,
              const A3 &a3,
              const A4 &a4,
              const A5 &a5,
              const A6 &a6,
              const A7 &a7)
        : m_functor(functor), m_a1(a1), m_a2(a2), m_a3(a3), m_a4(a4), m_a5(a5), m_a6(a6), m_a7(a7)
    {
    }

    FunctorType m_functor;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
    A4 m_a4;
    A5 m_a5;
    A6 m_a6;
    A7 m_a7;
};

template <typename FunctorType,
          typename RunType,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8>
class BindState<FunctorType, RunType, void(A1, A2, A3, A4, A5, A6, A7, A8)> : public BindStateBase {
public:
    typedef Invoker<BindState, RunType, 8> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;
    typedef UnwrapTraits<A1> Bound1UnwrapTraits;
    typedef UnwrapTraits<A2> Bound2UnwrapTraits;
    typedef UnwrapTraits<A3> Bound3UnwrapTraits;
    typedef UnwrapTraits<A4> Bound4UnwrapTraits;
    typedef UnwrapTraits<A5> Bound5UnwrapTraits;
    typedef UnwrapTraits<A6> Bound6UnwrapTraits;
    typedef UnwrapTraits<A7> Bound7UnwrapTraits;
    typedef UnwrapTraits<A8> Bound8UnwrapTraits;

    BindState(const FunctorType &functor,
              const A1 &a1,
              const A2 &a2,
              const A3 &a3,
              const A4 &a4,
              const A5 &a5,
              const A6 &a6,
              const A7 &a7,
              const A8 &a8)
        : m_functor(functor), m_a1(a1), m_a2(a2), m_a3(a3), m_a4(a4), m_a5(a5), m_a6(a6), m_a7(a7), m_a8(a8)
    {
    }

    FunctorType m_functor;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
    A4 m_a4;
    A5 m_a5;
    A6 m_a6;
    A7 m_a7;
    A8 m_a8;
};

template <typename FunctorType,
          typename RunType,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
class BindState<FunctorType, RunType, void(A1, A2, A3, A4, A5, A6, A7, A8, A9)> : public BindStateBase {
public:
    typedef Invoker<BindState, RunType, 9> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;
    typedef UnwrapTraits<A1> Bound1UnwrapTraits;
    typedef UnwrapTraits<A2> Bound2UnwrapTraits;
    typedef UnwrapTraits<A3> Bound3UnwrapTraits;
    typedef UnwrapTraits<A4> Bound4UnwrapTraits;
    typedef UnwrapTraits<A5> Bound5UnwrapTraits;
    typedef UnwrapTraits<A6> Bound6UnwrapTraits;
    typedef UnwrapTraits<A7> Bound7UnwrapTraits;
    typedef UnwrapTraits<A8> Bound8UnwrapTraits;
    typedef UnwrapTraits<A9> Bound9UnwrapTraits;

    BindState(const FunctorType &functor,
              const A1 &a1,
              const A2 &a2,
              const A3 &a3,
              const A4 &a4,
              const A5 &a5,
              const A6 &a6,
              const A7 &a7,
              const A8 &a8,
              const A9 &a9)
        : m_functor(functor), m_a1(a1), m_a2(a2), m_a3(a3), m_a4(a4), m_a5(a5), m_a6(a6), m_a7(a7), m_a8(a8), m_a9(a9)
    {
    }

    FunctorType m_functor;
    A1 m_a1;
    A2 m_a2;
    A3 m_a3;
    A4 m_a4;
    A5 m_a5;
    A6 m_a6;
    A7 m_a7;
    A8 m_a8;
    A9 m_a9;
};

// FunctorTraits<>
template <typename T>
struct FunctorTraits {
    typedef FunctorAdapter<T> FunctorType;
    typedef typename FunctorType::RunType RunType;
};

template <typename T>
struct FunctorTraits<Callback<T>> {
    typedef Callback<T> FunctorType;
    typedef typename Callback<T>::RunType RunType;
};

// FunctionTraits<>
//
// Unwrap a signature to get the return type and types of each parameter. There
// are ARITY FunctionTraits types.
template <typename Signature>
struct FunctionTraits;

template <typename R>
struct FunctionTraits<R()> {
    typedef R ReturnType;
};

template <typename R, typename A1>
struct FunctionTraits<R(A1)> {
    typedef R ReturnType;
    typedef A1 A1Type;
};

template <typename R, typename A1, typename A2>
struct FunctionTraits<R(A1, A2)> {
    typedef R ReturnType;
    typedef A1 A1Type;
    typedef A2 A2Type;
};

template <typename R, typename A1, typename A2, typename A3>
struct FunctionTraits<R(A1, A2, A3)> {
    typedef R ReturnType;
    typedef A1 A1Type;
    typedef A2 A2Type;
    typedef A3 A3Type;
};

template <typename R, typename A1, typename A2, typename A3, typename A4>
struct FunctionTraits<R(A1, A2, A3, A4)> {
    typedef R ReturnType;
    typedef A1 A1Type;
    typedef A2 A2Type;
    typedef A3 A3Type;
    typedef A4 A4Type;
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
struct FunctionTraits<R(A1, A2, A3, A4, A5)> {
    typedef R ReturnType;
    typedef A1 A1Type;
    typedef A2 A2Type;
    typedef A3 A3Type;
    typedef A4 A4Type;
    typedef A5 A5Type;
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct FunctionTraits<R(A1, A2, A3, A4, A5, A6)> {
    typedef R ReturnType;
    typedef A1 A1Type;
    typedef A2 A2Type;
    typedef A3 A3Type;
    typedef A4 A4Type;
    typedef A5 A5Type;
    typedef A6 A6Type;
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
struct FunctionTraits<R(A1, A2, A3, A4, A5, A6, A7)> {
    typedef R ReturnType;
    typedef A1 A1Type;
    typedef A2 A2Type;
    typedef A3 A3Type;
    typedef A4 A4Type;
    typedef A5 A5Type;
    typedef A6 A6Type;
    typedef A7 A7Type;
};

template <typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8>
struct FunctionTraits<R(A1, A2, A3, A4, A5, A6, A7, A8)> {
    typedef R ReturnType;
    typedef A1 A1Type;
    typedef A2 A2Type;
    typedef A3 A3Type;
    typedef A4 A4Type;
    typedef A5 A5Type;
    typedef A6 A6Type;
    typedef A7 A7Type;
    typedef A8 A8Type;
};

template <typename R,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
struct FunctionTraits<R(A1, A2, A3, A4, A5, A6, A7, A8, A9)> {
    typedef R ReturnType;
    typedef A1 A1Type;
    typedef A2 A2Type;
    typedef A3 A3Type;
    typedef A4 A4Type;
    typedef A5 A5Type;
    typedef A6 A6Type;
    typedef A7 A7Type;
    typedef A8 A8Type;
    typedef A9 A9Type;
};

}  // namespace detail

}  // namespace wsd

#endif
