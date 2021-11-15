#ifndef __URL_PARSE_H__
#define __URL_PARSE_H__

#include <string>

namespace wsd {

namespace detail {

struct Component {
    Component() : begin(0), len(-1)
    {
    }

    Component(int b, int l) : begin(b), len(l)
    {
    }

    int end() const
    {
        return begin + len;
    }
    bool isValid() const
    {
        return len >= 0;
    }

    void reset()
    {
        begin = 0;
        len = -1;
    }

    int begin;
    int len;  // <0 means this component does not exist.
};

// Construct a component given the begin and end positions. The end position is exclusive.
inline Component makeRange(int begin, int end)
{
    return Component(begin, end - begin);
}

inline std::string componentString(const char *url, const Component &component)
{
    if (component.isValid()) return std::string(url, component.begin, component.len);
    return std::string();
}

bool parseScheme(const char *s, int begin, int end, Component *scheme);
void parseAfterScheme(const char *s, int begin, int end, Component *authority, Component *full_path);
int countConsecutiveSlashes(const char *s, int begin, int end);
int findAuthorityTerminator(const char *s, int after_slashes, int end);
void parseAuthority(const char *s,
                    const Component &authority,
                    Component *username,
                    Component *password,
                    Component *host,
                    Component *port);
void parsePath(const char *s, const Component &full_path, Component *path, Component *query, Component *ref);
void parseUserInfo(const char *s, const Component &user_info, Component *username, Component *password);
void parseHostInfo(const char *s, const Component &host_info, Component *host, Component *port);

enum SpecialPort { PORT_UNSPECIFIED = -1, PORT_INVALID = -2 };

// Parse the port. Return the port if it is valid, or PORT_UNSPECIFIED if not
// exists, or PORT_INVALID if invalid.
int parsePort(const char *s, const Component &port);

bool isAuthorityTerminator(char c);

}  // namespace detail

}  // namespace wsd

#endif  // __URL_PARSE_H__
