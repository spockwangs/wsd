#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <string.h>
#include <exception>
#include <string>
#include <sstream>
#include "shared_ptr.h"
#include "wsd_assert.h"
#include "template_util.h"

namespace wsd {
    
    // Define the exception base of all exceptions throwed by this library.
    //
    // Note: The constructor and copy constructor should never throw an exception. If the
    // constructor throws an exception, you may report a different exception than intended
    // from a throw-expression that includs construction. If the copy constructor throws an
    // exception, std::terminate() will be called at the throw point.
    //
    // See also
    //   http://www.boost.org/community/error_handling.html
    //
    // Note: this class is not MT-safe.
    class WsdException : public std::exception {
    public:

        WsdException()  // never throw
            : m_filename(NULL),
              m_line(0),
              m_sep(':')
        {}
    
        WsdException(const char *filename, int line)  // never throw
            : m_filename(filename),
              m_line(line),
              m_sep(':')
        {}

        virtual ~WsdException() throw() {}
        
        virtual const char *what() const throw();
        
    private:

        /**
         * Returns the name of this exception. It is formatted in the what() message so
         * the catcher knows which exception it is catching.
         */
        virtual const char *name() const
        { return "WsdException"; }

        /**
         * Returns additional error info about the exception, and it will be formatted in
         * the what() message.
         */
        virtual std::string getErrorInfo() const
        { return ""; }

        const char *m_filename;
        int m_line;

        // Use SharedPtr<T> to ensure copy constructor throw no exceptions.
        mutable SharedPtr<std::string> m_what;  // lazily formatted what message
        mutable SharedPtr<std::string> m_data;  // to collect error info
        mutable char m_sep;                     // separator between error info

        template <typename T>
        void injectErrorInfo(const T& t) const;
        
        template <typename Exception, typename T>
        friend typename enable_if<is_derived_from<Exception, WsdException>, const Exception&>::type
        operator<<(const Exception& e, const T& t)
        {
            e.injectErrorInfo(t);
            return e;
        }
    };

    template <typename T>
    void WsdException::injectErrorInfo(const T& t) const
    {
        std::stringstream ss;
        ss << t;

        // Copy on write for `m_data'.
        if (!m_data.unique())
            m_data.reset(new std::string(*m_data));
        if (!m_data) {
            m_data.reset(new std::string(ss.str()));
        } else {
            (*m_data) += ss.str();
        }
    }

    namespace detail {
        
        // Convert errno to the description string.
        inline std::string errnoToString(int errno)
        {
            return strerror(errno);
        }

    }  // namespace detail
    
    class SyscallException : public WsdException {
    public:

        SyscallException(const char *filename, int line, int err)
            : WsdException(filename, line),
              m_errno(err)
        {}

        /**
         * Returns the error number of this system call which causes this exception.
         */
        int error() const
        { return m_errno; }

    protected:
        
        virtual const char *name() const
        { return "SyscallException"; }

        /**
         * Returns the error description of this error number.
         */
        virtual std::string getErrorInfo() const
        {
            return detail::errnoToString(m_errno);
        }

    private:

        int m_errno;  // the system call errno
    };
        
}  // namespace wsd

#endif  // __EXCEPTION_H__
