// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "wsd_exception.h"
#include <sstream>

namespace wsd {

    const char *WsdException::what() const throw()
    {
        try {
            // Copy on write for `m_what'.
            if (!m_what.unique())
                m_what.reset(new std::string(*m_what));
            if (!m_what || m_what->empty()) {
                std::stringstream ss;
                ss << name() << " thrown";
                if (m_filename && m_line > 0)
                    ss << " at " << m_filename << ':' << m_line;
                if (!m_what)
                    m_what.reset(new std::string());

                // Consume the message provided by derived class.
                const std::string& s = getErrorInfo();
                if (!s.empty()) {
                    // First error info is separated by ':'. The following errr info
                    // is separated by ';'.
                    ss << m_sep << ' ' << s;
                    m_sep = ';';
                }
                ss.str().swap(*m_what);
            }
                
            // Copy on write for `m_data'.
            if (!m_data.unique())
                m_data.reset(new std::string(*m_data));

            // Consume the error info if any.
            if (m_data && !m_data->empty()) {
                *m_what += m_sep;
                m_sep = ';';
                *m_what += ' ';
                m_what->append(*m_data);
                m_data.reset();
            }
            return m_what->c_str();
        } catch (...) {
            // do nothing
        }
        return "";
    }
    
}  // namespace wsd
