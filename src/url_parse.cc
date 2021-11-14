#include "detail/url_parse.h"

#include "wsd_assert.h"

namespace wsd {

namespace detail {

// Return true if a scheme is parsed in the given string of the given range.
bool parseScheme(const char *s, int begin, int end, Component *scheme)
{
    // Skip leading spaces.
    while (begin < end && std::isspace(s[begin])) ++begin;
    if (begin == end) return false;  // Input is empty or all whitespace.

    // Find the first colon character.
    for (int i = begin; i < end; i++) {
        if (s[i] == ':') {
            *scheme = makeRange(begin, i);
            return true;
        }
    }
    return false;  // No colon found: no scheme.
}

// Parse the string beyond the scheme of a standard URL and fill in all
// components. 's' is the whole URL being parsed, 'begin' points to the character
// immediately following the scheme and 'end' points beyond the last character of 's'.
//
// Input                Host             Path
// -----                -----------      ------
// http://foo.com/      "foo.com"        "/"
// http:foo.com/        "foo.com"        "/"
// http:///foo.com/     "foo.com"        "/"
// http://foo.com       "foo.com"        NULL
void parseAfterScheme(const char *s, int begin, int end, Component *authority, Component *full_path)
{
    int nslashes = countConsecutiveSlashes(s, begin, end);
    int after_slashes = begin + nslashes;

    // Split into two main parts, the authority (username, password, host, and
    // port) and the full path (path, query, and reference).
    int end_auth = findAuthorityTerminator(s, after_slashes, end);
    *authority = makeRange(after_slashes, end_auth);

    if (end_auth == end) {
        // No beginning of path found.
        *full_path = Component();
    } else {
        // Everything starting from the slash to the end is the path.
        *full_path = makeRange(end_auth, end);
    }
}

// Count consecutive slashes from 'begin' of 's'.
int countConsecutiveSlashes(const char *s, int begin, int end)
{
    int nslashes = 0;
    for (; begin < end && s[begin] == '/'; begin++) ++nslashes;
    return nslashes;
}

int findAuthorityTerminator(const char *s, int after_slashes, int end)
{
    for (int i = after_slashes; i < end; i++)
        if (isAuthorityTerminator(s[i])) return i;
    return end;  // not found
}

// Parse the authority and extract the username, password, host, and port.
void parseAuthority(const char *s,
                    const Component &authority,
                    Component *username,
                    Component *password,
                    Component *host,
                    Component *port)
{
    WSD_ASSERT(authority.isValid());
    if (authority.len == 0) {
        username->reset();
        password->reset();
        host->reset();
        port->reset();
        return;
    }

    // First split into two parts: user info (username and password), and host
    // info. Search backward for '@', which is the separator between user info and
    // host info.
    int i = authority.end();
    while (i >= authority.begin && s[i] != '@') --i;

    if (i < authority.begin) {
        // No user info, everything is host info.
        username->reset();
        password->reset();
        parseHostInfo(s, authority, host, port);
    } else {
        // Found user info: <user-info>@<host-info>.
        parseUserInfo(s, makeRange(authority.begin, i), username, password);
        parseHostInfo(s, makeRange(i + 1, authority.end()), host, port);
    }
}

// Pass the full path and extract the path, query and reference.
//
// path: /<segment1>/<segment2>/.../<segmentN>?<query>#<ref>
void parsePath(const char *s, const Component &full_path, Component *path, Component *query, Component *ref)
{
    if (!full_path.isValid()) {
        path->reset();
        query->reset();
        ref->reset();
        return;
    }
    WSD_ASSERT(full_path.len > 0);

    // Search for the first occurrence of either ? or #.
    int query_separator = -1;  // index of '?'
    int ref_separator = -1;    // index of '#'
    for (int i = full_path.begin; i < full_path.end(); i++) {
        switch (s[i]) {
        case '?':
            // Only match the query string if it preceds the reference fragment and
            // when we haven't found one already.
            if (ref_separator < 0 && query_separator < 0) query_separator = i;
            break;
        case '#':
            // Record the first '#' only.
            if (ref_separator < 0) ref_separator = i;
            break;
        }
    }

    // Markers pointing to the character after each of these corresponding components.
    int path_end, query_end;

    // Ref fragment: from '#' to the end of the full path.
    if (ref_separator >= 0) {
        *ref = makeRange(ref_separator + 1, full_path.end());
        path_end = query_end = ref_separator;
    } else {
        ref->reset();
        path_end = query_end = full_path.end();
    }

    // Query fragment: from '?' to the ref fragment or the end of the full path.
    if (query_separator >= 0) {
        *query = makeRange(query_separator + 1, query_end);
        path_end = query_separator;
    } else {
        query->reset();
    }

    // Path: everything from beginning of the full path to the next boundary (either
    // the query fragment, or the ref fragment, or the end of the full path).
    if (path_end > full_path.begin)
        *path = makeRange(full_path.begin, path_end);
    else
        path->reset();
}

// Parse the user info and extract the username password.
void parseUserInfo(const char *s, const Component &user_info, Component *username, Component *password)
{
    WSD_ASSERT(user_info.isValid());

    // Find the first colon in the user info, which separates the username and
    // password.
    int colon_offset = user_info.begin;
    while (colon_offset < user_info.end() && s[colon_offset] != ':') ++colon_offset;

    if (colon_offset < user_info.end()) {
        // Found colon: <username>:<password>.
        *username = makeRange(user_info.begin, colon_offset);
        *password = makeRange(colon_offset + 1, user_info.end());
    } else {
        // No separator, everything is the username.
        *username = user_info;
        password->reset();
    }
}

// Parse the host info and extract the host and port.
void parseHostInfo(const char *s, const Component &host_info, Component *host, Component *port)
{
    WSD_ASSERT(host_info.isValid());

    // If the host starts with a left-bracket, assume the entire host is an IPv6
    // literal. Otherwise, assume none of the host is an IPv6 literal. This assumption
    // will be overriden if we find a right-bracket.
    int ipv6_terminator = s[host_info.begin] == '[' ? host_info.end() : -1;
    int colon_offset = -1;

    // Find the last right-bracket, and the last colon.
    for (int i = host_info.begin; i < host_info.end(); i++) {
        switch (s[i]) {
        case ']':
            ipv6_terminator = i;
            break;
        case ':':
            colon_offset = i;
            break;
        }
    }

    if (colon_offset > ipv6_terminator) {
        // Found a port: <hostname>:<port>
        *host = makeRange(host_info.begin, colon_offset);
        if (host->len == 0) host->reset();
        *port = makeRange(colon_offset + 1, host_info.end());
    } else {
        // No port, everything in host info is hostname.
        *host = host_info;
        port->reset();
    }
}

int parsePort(const char *s, const Component &port)
{
    if (port.len <= 0) return PORT_UNSPECIFIED;

    int port_num = std::atoi(componentString(s, port).c_str());
    if (port_num <= 0) return PORT_INVALID;
    return port_num;
}

bool isAuthorityTerminator(char c)
{
    return c == '/' || c == '?' || c == '#';
}

}  // namespace detail

}  // namespace wsd
