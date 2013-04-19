#ifndef __ASSERT_H__
#define __ASSERT_H__

#include <cstdlib>            // for abort()
#include <iostream>

namespace wsd {

    inline void assertion_failed(const char *expr, const char *function, const char *file, int line)
    {
        std::cerr << "at " << file << ':' << line << ": in " << function
                  << "(): Assertion `" << expr << "' failed" << std::endl;
        std::abort();
    }
}

#define WSD_ASSERT(expr)                                                \
    ((expr) ? ((void)0) : ::wsd::assertion_failed(#expr, __func__, __FILE__, __LINE__))

namespace wsd {
    
    template<bool> struct CompileTimeError;
    template<> struct CompileTimeError<true> {};

}  // namespace wsd

#define STATIC_ASSERT(exp, msg)                                         \
    { wsd::CompileTimeError<((exp) != 0)> ERROR_##msg; (void) ERROR_##msg; }

#endif
