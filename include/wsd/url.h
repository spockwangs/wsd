#ifndef __URL_H__
#define __URL_H__

#include <cstdlib>
#include <string>

#include "wsd/detail/url_canon.h"
#include "wsd/detail/url_parse.h"

namespace wsd {

class Url {
public:
    enum SpecialPort { PORT_UNSPECIFIED = detail::PORT_UNSPECIFIED, PORT_INVALID = detail::PORT_INVALID };

    Url() : m_is_valid(false)
    {
    }

    explicit Url(const std::string& s) : m_is_valid(false)
    {
        parse(s.c_str(), s.length());
        canonicalize(s.c_str(), s.length());
    }

    bool isValid() const
    {
        return m_is_valid;
    }

    std::string getScheme() const
    {
        return detail::componentString(m_canon_url.c_str(), m_scheme);
    }

    std::string getUsername() const
    {
        return detail::componentString(m_canon_url.c_str(), m_username);
    }

    std::string getPassword() const
    {
        return detail::componentString(m_canon_url.c_str(), m_password);
    }

    std::string getHost() const
    {
        return detail::componentString(m_canon_url.c_str(), m_host);
    }

    std::string getPort() const
    {
        return detail::componentString(m_canon_url.c_str(), m_port);
    }

    // Returns a parsed version of the port. Returns PORT_UNSPECIFIED if the port is
    // not specified, or PORT_INVALID if the port is invalid.
    int getIntPort() const
    {
        return detail::parsePort(m_canon_url.c_str(), m_port);
    }

    // Returns the port number of the url, or default port number if the port is not
    // specified. If the scheme has no concept of port returns PORT_UNSPECIFIED.
    int getEffectiveIntPort() const
    {
        int port = getIntPort();
        if (port == PORT_UNSPECIFIED && m_scheme.isValid())
            return defaultPortForScheme(&m_canon_url[m_scheme.begin], m_scheme.len);
        return port;
    }

    std::string getPath() const
    {
        return detail::componentString(m_canon_url.c_str(), m_path);
    }

    std::string getQuery() const
    {
        return detail::componentString(m_canon_url.c_str(), m_query);
    }

    std::string getRef() const
    {
        return detail::componentString(m_canon_url.c_str(), m_ref);
    }

    bool hasScheme() const
    {
        return m_scheme.isValid();
    }
    bool hasUsername() const
    {
        return m_username.isValid();
    }
    bool hasPassword() const
    {
        return m_password.isValid();
    }
    bool hasHost() const
    {
        return m_host.isValid();
    }
    bool hasPort() const
    {
        return m_port.isValid();
    }
    bool hasPath() const
    {
        return m_path.isValid();
    }
    bool hasQuery() const
    {
        return m_query.isValid();
    }
    bool hasRef() const
    {
        return m_ref.isValid();
    }

    std::string getCanonUrl() const
    {
        return m_canon_url;
    }

private:
    // Parse the input and fill in various components from the input.
    void parse(const char* input, int len);

    // Canonicalize the parsed comonents and refill them.
    void canonicalize(const char* input, int len);

    static void trim(const char* s, int* begin, int* end);

    static int defaultPortForScheme(const char* scheme, int len);

    // Define equality operator.
    bool operator==(const Url& other) const
    {
        return m_canon_url == other.m_canon_url;
    }

    bool operator!=(const Url& other) const
    {
        return m_canon_url != other.m_canon_url;
    }

    // Allows Url to be used as a key in STL.
    bool operator<(const Url& other) const
    {
        return m_canon_url < other.m_canon_url;
    }

    bool m_is_valid;
    std::string m_canon_url;  // canonicalized URL (possiblly invalid)
    detail::Component m_scheme;
    detail::Component m_username;
    detail::Component m_password;
    detail::Component m_host;
    detail::Component m_port;
    detail::Component m_path;
    detail::Component m_query;
    detail::Component m_ref;

    friend std::ostream& operator<<(std::ostream& os, const Url& url)
    {
        return os << url.m_canon_url;
    }
};

}  // namespace wsd

#endif  // __URL_H__
