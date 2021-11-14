#include "url.h"

#include <cstring>

#include "wsd_assert.h"

namespace wsd {

void Url::parse(const char *s, int len)
{
    WSD_ASSERT(s != NULL);
    WSD_ASSERT(len >= 0);

    // Trim the leading and tailing spaces.
    int begin = 0;
    trim(s, &begin, &len);

    int after_scheme;
    if (detail::parseScheme(s, begin, len, &m_scheme))
        after_scheme = m_scheme.end() + 1;  // skip past the colon
    else
        after_scheme = begin;

    // Extract authority and full path, and then parse those two sub-parts.
    detail::Component authority, full_path;
    detail::parseAfterScheme(s, after_scheme, len, &authority, &full_path);
    detail::parseAuthority(s, authority, &m_username, &m_password, &m_host, &m_port);
    detail::parsePath(s, full_path, &m_path, &m_query, &m_ref);

    // Check if the standard URL is valid.
    // A valid URL:
    //   * must have non-empty scheme;
    //   * must have non-empty host;
    //   * path either exists and non-empty, or does not exist
    if (m_scheme.len > 0 && m_host.len > 0 && m_path.len != 0)
        m_is_valid = true;
    else
        m_is_valid = false;
}

void Url::canonicalize(const char *input, int len)
{
    // Allocate enough space and hopefully we will not allocate again.
    m_canon_url.reserve(len + 10);

    // Output scheme.
    detail::canonicalizeScheme(input, len, &m_scheme, &m_canon_url);

    // Output authority.
    bool have_authority;
    if (m_username.isValid() || m_password.isValid() || m_host.isValid() || m_port.isValid()) {
        have_authority = true;

        // Only write the authority separators when we have a scheme.
        if (m_scheme.isValid()) m_canon_url += "//";

        // User info: the canonicalizer will handle the : and @
        detail::canonicalizeUserInfo(input, len, &m_username, &m_password, &m_canon_url);

        int default_port = defaultPortForScheme(&m_canon_url.c_str()[m_scheme.begin], m_scheme.len);
        detail::canonicalizeHostInfo(input, len, default_port, &m_host, &m_port, &m_canon_url);
    } else {
        // No authority.
        have_authority = false;
    }

    // Path
    if (m_path.isValid()) {
        canonicalizePath(input, len, &m_path, &m_canon_url);
    } else if (have_authority || m_query.isValid() || m_ref.isValid()) {
        // When we have an empty path, make up a path when we have an authority or
        // something following the path.
        m_path = detail::Component(m_canon_url.length(), 1);
        m_canon_url += '/';
    }  // else: No path at all.

    // Query
    canonicalizeQuery(input, len, &m_query, &m_canon_url);

    // Ref
    canonicalizeRef(input, len, &m_ref, &m_canon_url);
}

// Trim the leading and trailing spaces of given string in the given range and adjust
// the range accordingly.
void Url::trim(const char *s, int *begin, int *end)
{
    WSD_ASSERT(*begin <= *end);

    while (*begin < *end && std::isspace(s[*begin])) ++*begin;
    while (*end > *begin && std::isspace(s[*end - 1])) --*end;
}

int Url::defaultPortForScheme(const char *scheme, int len)
{
    int default_port = PORT_UNSPECIFIED;
    switch (len) {
    case 3:
        if (std::strncmp(scheme, "ftp", len) == 0) default_port = 21;
        break;
    case 4:
        if (std::strncmp(scheme, "http", len) == 0) default_port = 80;
        break;
    case 5:
        if (std::strncmp(scheme, "https", len) == 0) default_port = 443;
        break;
    }
    return default_port;
}

}  // namespace wsd
