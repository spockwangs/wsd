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

    Entity(Entity&&) = default;
    Entity& operator=(Entity&&) = default;

    virtual std::string GetId() const = 0;
};

class LineItem : public Entity {
public:
    LineItem(const std::string& item_id, const std::string& name, int price)
        : item_id_(item_id), name_(name), price_(price)
    {
    }

    std::string GetId() const override
    {
        return item_id_;
    }

    std::string GetName() const
    {
        return name_;
    }

    int GetPrice() const
    {
        return price_;
    }

private:
    std::string item_id_;
    std::string name_;
    int price_ = 0;
};

class Order : public Entity {
public:
    Order() = default;

    Order(const std::string& id) : id_(id)
    {
    }

    Order(Order&&) = default;

    static Order MakeOrder(const std::string& id, std::vector<LineItem>&& line_items)
    {
        Order result{id};
        result.line_items_ = std::move(line_items);
        for (const auto& item : line_items) {
            result.total_price_ += item.GetPrice();
        }
        return std::move(result);
    }

    ~Order() = default;

    std::string GetId() const override
    {
        return id_;
    }

    int GetTotalPrice() const
    {
        return total_price_;
    }

    void AddLineItem(const std::string& name, int price)
    {
        line_items_.emplace_back(GenLineItemId(), name, price);
        total_price_ += price;
    }

    const std::vector<LineItem>& GetLineItems() const
    {
        return line_items_;
    }

    size_t GetNumOfLineItems() const
    {
        return line_items_.size();
    }

private:
    std::string GenLineItemId()
    {
        return std::to_string(line_items_.size());
    }

    std::string id_;
    int total_price_ = 0;
    std::vector<LineItem> line_items_;
};

}  // namespace ddd
