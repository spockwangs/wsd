#include "detail/url_canon.h"

#include <sstream>

#include "detail/url_parse.h"
#include "wsd_assert.h"

namespace wsd {

    namespace detail {
        
        namespace {
            
            enum {
                // The dot is part of a filename and not special.
                NOT_DIRECTORY,

                // A signle dot representing current directory.
                CUR_DIRECTORY,

                // The dot is the first of a double-dot.
                DIRECTORY_UP
            };
            
            // When the path resolver finds a dot, this function is called with the
            // character following that dot to see what it is. The return value
            // indicates what type this dot is (see above). This code handles the case
            // where the dot is at the end of the input.
            //
            // *consumed_len will contain number of characters in the input which express
            // what we found.
            int classifyAfterDot(const char *input, int after_dot, int end, int *consumed_len)
            {
                if (after_dot == end) {
                    // Single dot at the end.
                    *consumed_len = 0;
                    return CUR_DIRECTORY;
                }

                if (input[after_dot] == '/') {
                    // Single dot followed by a slash.
                    *consumed_len = 1;
                    return CUR_DIRECTORY;
                }

                if (input[after_dot] == '.') {
                    if (after_dot + 1 == end) {
                        // Double dot at the end.
                        *consumed_len = 2;
                        return DIRECTORY_UP;
                    }
                    if (input[after_dot + 1] == '/') {
                        // Double dot followed by a slash.
                        *consumed_len = 2;
                        return DIRECTORY_UP;
                    }
                }

                // The dots are followed by something else, not special.
                *consumed_len = 0;
                return NOT_DIRECTORY;
            }

            // Rewinds the output to the previous slash. It is assumed that the output
            // ends with a slash.
            void backUpToPreviousSlash(int path_begin_in_output, std::string *output)
            {
                WSD_ASSERT(!output->empty());

                int i = output->length()-1;
                WSD_ASSERT(output->at(i) == '/');
                if (i == path_begin_in_output)
                    return;

                // Now back up until we find another slash.
                --i;
                while (output->at(i) != '/' && i > path_begin_in_output)
                    --i;

                // Shrink the output to just include the slash we just found.
                output->resize(i+1);
            }

        }  // namespace

        void canonicalizeScheme(const char *input, int len, Component *scheme, std::string *output)
        {
            WSD_ASSERT(scheme->end() <= len);
            
            if (scheme->isValid()) {
                for (int i = scheme->begin; i < scheme->end(); i++)
                    *output += std::tolower(input[i]);
                *output += ':';
            }
        }

        void canonicalizeUserInfo(const char *input, int len, Component *username,
                                  Component *password, std::string *output)
        {
            WSD_ASSERT(username->end() <= len);
            WSD_ASSERT(password->end() <= len);

            // Output the username and password if any.
            if (username->isValid()) {
                *output += componentString(input, *username);
                *username = makeRange(output->length() - username->len, output->length());
                if (password->isValid()) {
                    *output += ':';
                    *output += componentString(input, *password);
                    *password = makeRange(output->length() - password->len, output->length());
                }
                *output += '@';
            }
        }

        void canonicalizeHostInfo(const char *input, int len, int default_port, Component *host,
                                  Component *port, std::string *output)
        {
            WSD_ASSERT(host->end() <= len);
            WSD_ASSERT(port->end() <= len);

            // Host
            *output += componentString(input, *host);
            *host = makeRange(output->length() - host->len, output->length());

            // Port
            int port_num = parsePort(input, *port);
            if (port_num == PORT_UNSPECIFIED || port_num == default_port) {
                port->reset();
            } else if (port_num == PORT_INVALID) {
                // Invalid port: copy the text from input so the user can see the error.
                *output += ':';
                *output += componentString(input, *port);
                *port = makeRange(output->length() - port->len, output->length());
            } else {
                // Convert port number back to an integer.
                *output += ':';
                int port_begin = output->length();
                std::stringstream ss;
                ss << port_num;
                *output += ss.str();
                *port = makeRange(port_begin, output->length());
            }
        }
        
        void canonicalizePath(const char *input, int len, Component *path, std::string *output)
        {
            WSD_ASSERT(path->isValid());
            WSD_ASSERT(path->end() <= len);

            // Extract the segments from the path and skip the unnecessary segments.
            // path = /<seg1>/<seg2>/.../<segN>
            WSD_ASSERT(input[path->begin] == '/');
            int path_begin_in_output = output->length();
            for (int i = path->begin; i < path->end(); i++) {
                int ch = input[i];
                if (ch == '.') {
                    // See if the dot is preceded by a slash.
                    // Note when canonicalizing paths they always start with a slash
                    // instead of a dot.
                    WSD_ASSERT(output->length() > 0);
                    if (output->at(output->length()-1) == '/') {
                        int consumed_len;
                        switch (classifyAfterDot(input, i+1, path->end(), &consumed_len)) {
                        case NOT_DIRECTORY:
                            // Nothing special.
                            *output += '.';
                            break;
                        case CUR_DIRECTORY:
                            // skip the input
                            i += consumed_len;
                            break;
                        case DIRECTORY_UP:
                            backUpToPreviousSlash(path_begin_in_output, output);
                            i += consumed_len;
                            break;
                        }
                    } else {
                        *output += '.';
                    }
                } else if (ch == '/') {
                    // Append it if not preceded by a slash in the output. Multipe slashes
                    // are collapsed to a single slash.
                    if (output->empty() || output->at(output->length()-1) != '/')
                        *output += ch;
                } else {
                    // Nothing special, just append it.
                    *output += ch;
                }
            }
            *path = makeRange(path_begin_in_output, output->length());
        }

        void canonicalizeQuery(const char *input, int len, Component *query, std::string *output)
        {
            WSD_ASSERT(query->end() <= len);
            
            // Output query if any.
            if (query->isValid()) {
                *output += '?';
                *output += componentString(input, *query);
                *query = makeRange(output->length() - query->len, output->length());
            }
        }

        void canonicalizeRef(const char *input, int len, Component *ref, std::string *output)
        {
            WSD_ASSERT(ref->end() <= len);
            
            // Output reference fragment if any.
            if (ref->isValid()) {
                *output += '#';
                *output += componentString(input, *ref);
                *ref = makeRange(output->length() - ref->len, output->length());
            }
        }

    }  // namespace detail

}  // namespace wsd
