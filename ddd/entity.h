// Copyright (c) 2023 Tencent
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#pragma once

#include <string>

#include "repository.h"

namespace ddd {

class Order : public Entity {
public:
    Order() = default;

    Order(const std::string& id, int price) : id_(id), price_(price)
    {
    }

    ~Order() = default;

    std::string GetId() const override
    {
        return id_;
    }

    int GetPrice() const
    {
        return price_;
    }

    void SetPrice(int price)
    {
        price_ = price;
    }

private:
    std::string id_;
    int price_ = 0;
};

}  // namespace ddd
