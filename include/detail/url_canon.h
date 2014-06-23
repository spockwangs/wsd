#ifndef __URL_CANON_H__
#define __URL_CANON_H__

#include <string>

namespace wsd {

    namespace detail {

        // forward declaration
        struct Component;
        
        void canonicalizeScheme(const char *input, int len, Component *scheme, std::string *output);
        void canonicalizePath(const char *input, int len, Component *path, std::string *output);
        void canonicalizeQuery(const char *input, int len, Component *query, std::string *output);
        void canonicalizeRef(const char *input, int len, Component *ref, std::string *output);
        void canonicalizeUserInfo(const char *input, int len, Component *username,
                                  Component *password, std::string *output);
        void canonicalizeHostInfo(const char *input, int len, int default_port, Component *host,
                                  Component *port, std::string *output);
    }  // namespace detail

}  // namespace wsd

#endif  // __URL_CANON_H__
