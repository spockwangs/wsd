#include "template_util.h"
#include "gtest/gtest.h"
#include "wsd_assert.h"
#include <string>

using namespace std;

TEST(template_util, is_non_const_reference)
{
    STATIC_ASSERT(!wsd::is_non_const_reference<int>::value, should_be_non_const_reference);
    STATIC_ASSERT(wsd::is_non_const_reference<int&>::value, should_be_non_const_reference);
    STATIC_ASSERT(!wsd::is_non_const_reference<const int&>::value, should_be_non_const_reference);
}

TEST(template_util, is_raw_pointer)
{
    STATIC_ASSERT(!wsd::is_raw_pointer<int>::value, should_not_be_raw_pointer);
    STATIC_ASSERT(wsd::is_raw_pointer<int*>::value, should_be_raw_pointer);
    STATIC_ASSERT(wsd::is_raw_pointer<void*>::value, should_be_raw_pointer);
}

TEST(template_util, is_convertible)
{
    // Note: without the inner parentheses the colon will be considered as the separator
    // of macro parameters, and the compiler will believe we passed 3 parameters to "STATIC_ASSERT".
    STATIC_ASSERT((wsd::is_convertible<int, long>::value), can_convert_int_long);
    STATIC_ASSERT((wsd::is_convertible<long, int>::value), can_convert_long_int);
    STATIC_ASSERT(!(wsd::is_convertible<int*, long*>::value), cant_convert_int_long_pointer);
    STATIC_ASSERT((wsd::is_convertible<int*, void*>::value), cant_convert_int_void_pointer);
    STATIC_ASSERT(!(wsd::is_convertible<void*, int*>::value), cant_convert_void_int_pointer);
}

// Local types can't be used as template argument.
enum E { A, B };

TEST(template_util, is_class)
{
    STATIC_ASSERT(!wsd::is_class<int>::value, int_is_not_class);
    STATIC_ASSERT(!wsd::is_class<void>::value, void_is_not_class);
    STATIC_ASSERT(!wsd::is_class<float>::value, float_is_not_class);
    STATIC_ASSERT(!wsd::is_class<double>::value, double_is_not_class);
    STATIC_ASSERT(!wsd::is_class<bool>::value, bool_is_not_class);
    STATIC_ASSERT(!wsd::is_class<E>::value, enum_is_not_class);
    STATIC_ASSERT(wsd::is_class<string>::value, string_is_class);
    STATIC_ASSERT(wsd::is_class<vector<int> >::value, vector_is_class);
}

struct Bar {
    void bar();
};

struct Foo {
    void foo();
};

struct Foo2 : private Foo {};
struct Foo3 : public Foo2 {};

TEST(template_util, has_method_foo)
{
    STATIC_ASSERT(!wsd::has_method_foo<Bar>::value, bar_dont_has_foo);
    STATIC_ASSERT(wsd::has_method_foo<Foo>::value, foo_has_foo);
    STATIC_ASSERT(wsd::has_method_foo<Foo2>::value, foo2_has_foo);
    STATIC_ASSERT(wsd::has_method_foo<Foo3>::value, foo3_has_foo);
}

TEST(template_util, is_same_type)
{
    STATIC_ASSERT((wsd::is_same_type<int, int>::value), same_type);
    STATIC_ASSERT(!(wsd::is_same_type<int, const int>::value), not_same_type);
    STATIC_ASSERT(!(wsd::is_same_type<int, volatile int>::value), not_same_type);
    STATIC_ASSERT((!wsd::is_same_type<int, const volatile int>::value), not_same_type);
    STATIC_ASSERT((!wsd::is_same_type<int*, const int*>::value), not_same_type);    
    STATIC_ASSERT((!wsd::is_same_type<int*, int*const>::value), not_same_type);    
    STATIC_ASSERT((!wsd::is_same_type<int*, const int*const>::value), not_same_type);    
    STATIC_ASSERT((!wsd::is_same_type<const int*, const int*const>::value), not_same_type);    
    STATIC_ASSERT((!wsd::is_same_type<int*const, const int*const>::value), not_same_type);    
    STATIC_ASSERT((!wsd::is_same_type<void, int>::value), not_same_type);    
    STATIC_ASSERT((!wsd::is_same_type<void, void*>::value), not_same_type);    
    STATIC_ASSERT((!wsd::is_same_type<void*, int*>::value), not_same_type);    
    STATIC_ASSERT((!wsd::is_same_type<const void*, void*>::value), not_same_type);    
    STATIC_ASSERT((!wsd::is_same_type<const void*, void*>::value), not_same_type);
    STATIC_ASSERT((!wsd::is_same_type<void (*)(), void (*)(int)>::value), not_same_type);
    STATIC_ASSERT((wsd::is_same_type<void (*)(), void (*)(void)>::value), same_type);
    STATIC_ASSERT((!wsd::is_same_type<int (*)(), void (*)()>::value), not_same_type);
    STATIC_ASSERT((!wsd::is_same_type<int (*)(), long (*)()>::value), not_same_type);
    STATIC_ASSERT((wsd::is_same_type<int (*)() throw(), int (*)()>::value), not_same_type);
    STATIC_ASSERT((wsd::is_same_type<int (*)() throw(), int (*)() throw(int)>::value), not_same_type);
}

struct X {};
struct Y {};
struct Z : X {};

TEST(template_util, is_derived_from)
{
    STATIC_ASSERT((!wsd::is_derived_from<int, int>::value), int_not_derived_from_int);
    STATIC_ASSERT((!wsd::is_derived_from<int*, int*>::value), int_pointer_not_derived_from_int_pointer);
    STATIC_ASSERT((!wsd::is_derived_from<void*, int*>::value), void_pointer_not_derived_from_int_pointer);
    STATIC_ASSERT((!wsd::is_derived_from<X, Y>::value), X_not_derived_from_Y);
    STATIC_ASSERT((!wsd::is_derived_from<Y, X>::value), Y_not_derived_from_X);
    STATIC_ASSERT((!wsd::is_derived_from<X, Z>::value), X_not_derived_from_Z);
    STATIC_ASSERT((wsd::is_derived_from<Z, X>::value), Z_derived_from_X);
    STATIC_ASSERT((wsd::is_derived_from<X, X>::value), X_derived_from_X);
}
