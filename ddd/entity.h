// Copyright (c) 2023 Tencent
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#pragma once

#include <string>

namespace ddd {

class Entity {
public:
    Entity() = default;

    virtual ~Entity() = default;

    // Entity should not be copied. We should not have two copies of the same entity in one session.
    Entity(const Entity&) = delete;
    void operator=(const Entity&) = delete;

    virtual std::string GetId() const = 0;
};


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
