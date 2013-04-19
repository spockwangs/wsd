// Copyright (c) 2013 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "tick_count.h"
#include "gtest/gtest.h"

TEST(tick_count, tick_count)
{
    wsd::TickCount t0 = wsd::TickCount::now();
    sleep(1);
    wsd::TickCount t1 = wsd::TickCount::now();
    EXPECT_NEAR(1000.0, (t1-t0).milliseconds(), 1);
}

TEST(interval, constructor)
{
    wsd::TickCount::Interval i;
    EXPECT_DOUBLE_EQ(0, i.milliseconds());
}

TEST(interval, add_and_sub)
{
    wsd::TickCount::Interval i(3.0), j(5.0);
    EXPECT_DOUBLE_EQ(8.0, (i+j).milliseconds());
    EXPECT_DOUBLE_EQ(2.0, (j-i).milliseconds());

    i += wsd::TickCount::Interval(4.0);
    EXPECT_DOUBLE_EQ(7.0, i.milliseconds());

    j -= wsd::TickCount::Interval(2.0);
    EXPECT_DOUBLE_EQ(3.0, j.milliseconds());
}
    
