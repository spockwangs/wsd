// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "repository_impl.h"
#include <gtest/gtest.h>

TEST(OrderRepository, Basic) {
    OrderRepository repo{};
    EXPECT_EQ(repo.Find("a"), nullptr);
}

