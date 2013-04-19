// Copyright (c) 2012 wbb.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "checked_delete.h"
#include "gtest/gtest.h"

TEST(checked_delete, delete_int)
{
    int *p = new int;
    wsd::checked_delete(p);
}

class X;
X *makeX();

// The following code should not pass the compilation.
TEST(checked_delete, delete_incomplete_type)
{
    // X *p = makeX();
    // wsd::checked_delete(p);
}

X *make_x_array();

// The following code should not pass the compilation.
TEST(checked_array_delete, delete_incomplete_type)
{
    // X *p = make_x_array();
    // wsd::checked_array_delete(p);
}
