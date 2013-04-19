#ifndef __TEMPLATE_UTIL_H__
#define __TEMPLATE_UTIL_H__

namespace wsd {

    template <typename T, T v>
    struct integral_constant {
        typedef T value_type;
        typedef integral_constant<T, v> type;
        static const T value = v;
    };

    template <typename T, T v> const T integral_constant<T, v>::value;
    
    typedef integral_constant<bool, true> true_type;
    typedef integral_constant<bool, false> false_type;

    template <typename T> struct is_void : false_type {};
    template <> struct is_void<void> : true_type {};
    
    template <typename T> struct is_non_const_reference : false_type {};
    template <typename T> struct is_non_const_reference<T&> : true_type {};
    template <typename T> struct is_non_const_reference<const T&> : false_type {};

    template <typename T> struct is_raw_pointer : false_type {};
    template <typename T> struct is_raw_pointer<T*> : true_type {};

    // Select type1 if true, type2 otherwise.
    template <bool, typename type1, typename type2>
    struct select_type_c;

    template <typename type1, typename type2>
    struct select_type_c<true, type1, type2> {
        typedef type1 type;
    };

    template <typename type1, typename type2>
    struct select_type_c<false, type1, type2> {
        typedef type2 type;
    };

    // Select T1 if Cond::value is true.
    template <typename Cond, typename T1, typename T2>
    struct select_type : select_type_c<Cond::value, T1, T2> {};
    
    namespace detail {

        // Types Yes and No are guaranteed such that sizeof(Yes) != sizeof(No). The reason
        // why we declare Yes and No as a type of reference-to-array is because functions
        // can not return array types.
        typedef char (&Yes)[1];
        typedef char (&No)[2];

        // Used to determine if a type From can convert to a type To. See Alexandrescu's
        // "Modern C++ Design" for more details on this trick.
        struct ConvertHelper {
            template <typename To>
            static Yes Test(To);

            template <typename To>
            static No Test(...);
            
            template <typename From>
            static From Make();
        };

        // Used to determine if a type is a struct/class/union.
        // See http://src.chromium.org/viewvc/chrome/trunk/src/base/template_util.h?content-type=text%2Fplain
        struct IsClassHelper {
            template <typename C>
            static Yes Test(void (C::*)(void));

            template <typename C>
            static No Test(...);
        };

    }  // namespace detail

    // Inherits from true_type if From is convertible to To, false_type otherwise.
    // Note: From and To must be not abstract type.
    template <typename From, typename To>
    struct is_convertible
            : integral_constant<bool,
                                sizeof(detail::ConvertHelper::Test<To>(
                                               detail::ConvertHelper::Make<From>())) == sizeof(detail::Yes)> {
    };
    
    template <typename T>
    struct is_class
            : integral_constant<bool,
                                sizeof(detail::IsClassHelper::Test<T>(0)) == sizeof(detail::Yes)> {
    };

    // Check if a type is derived from another type.
    template <typename D, typename B>
    struct is_derived_from {
        enum { value = is_class<D>::value && is_class<B>::value && is_convertible<D*, B*>::value };
    };
    
    // Check if two types are the same
    template <typename U, typename V>
    struct is_same_type : integral_constant<bool, false> {};

    template <typename U>
    struct is_same_type<U, U> : integral_constant<bool, true> {};
    
    // Use SFINAE to check if a class type has the specified member function. Note this
    // trick only checks its existence, not its signature.
    //
    // See also:
    //   http://en.wikipedia.org/wiki/Substitution_failure_is_not_an_error
    //   http://stackoverflow.com/questions/1966362/sfinae-to-check-for-inherited-member-functions
    template <typename T>
    class has_method_foo {

        struct BasMixin {
            void foo();
        };

        struct Base : public T, public BasMixin {};

        template <void (BasMixin::*)(void)> struct Helper {};
        
        template <typename C>
        static detail::No Test(Helper<&C::foo>*);

        template <typename>
        static detail::Yes Test(...);

    public:

        enum { value = sizeof(Test<Base>(0)) == sizeof(detail::Yes) };
    };

    // The `enable_if' family of templates is used to control the inclusion or exclusion
    // from a set of matching functions or specializations during overload resolution.
    template <bool, typename T = void>
    struct enable_if_c {
        typedef T type;
    };

    template <typename T>
    struct enable_if_c<false, T> {};

    template <typename Cond, typename T = void>
    struct enable_if : public enable_if_c<Cond::value, T> {};

    template <bool, typename T = void>
    struct disable_if_c {
        typedef T type;
    };

    template <typename T>
    struct disable_if_c<true, T> {};

    template <typename Cond, typename T = void>
    struct disable_if : public disable_if_c<Cond::value, T> {};
    
} // namespace wsd

#endif  // __TEMPLATE_UTIL_H__
