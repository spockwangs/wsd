// Copyright (c) 2013 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#ifndef __TICK_COUNT_H__
#define __TICK_COUNT_H__

#include <time.h>

namespace wsd {

    class TickCount {
    public:
        class Interval;
        static TickCount now();

    private:
        TickCount(const struct timespec& tm)
            : m_milliseconds((double) tm.tv_sec * 1000 + tm.tv_nsec / 1E6)
        {}
        
        double m_milliseconds;
        friend TickCount::Interval operator-(const TickCount& t1, const TickCount& t0);
    };

    class TickCount::Interval {
    public:
        Interval()
            : m_milliseconds(0)
        {}
        
        explicit Interval(double milliseconds)
            : m_milliseconds(milliseconds)
        {}
        
        double milliseconds() const
        {
            return m_milliseconds;
        }
        
        Interval operator+=(const Interval& i)
        {
            m_milliseconds += i.m_milliseconds;
            return *this;
        }

        Interval operator-=(const Interval& i)
        {
            m_milliseconds -= i.m_milliseconds;
            return *this;
        }

    private:
        double m_milliseconds;

        friend TickCount::Interval operator+(const TickCount::Interval& i, const TickCount::Interval& j)
        {
            return TickCount::Interval(i.m_milliseconds + j.m_milliseconds);
        }
        
        friend TickCount::Interval operator-(const TickCount::Interval& i, const TickCount::Interval& j)
        {
            return TickCount::Interval(i.m_milliseconds - j.m_milliseconds);
        }
    };
    
    /**
     * \pre 't1' must occur after 't0'
     */
    inline TickCount::Interval operator-(const TickCount& t1, const TickCount& t0)
    {
        return TickCount::Interval(t1.m_milliseconds - t0.m_milliseconds);
    }

}  // wsd

#endif  // __TICK_COUNT_H__
