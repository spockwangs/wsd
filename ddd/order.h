// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include <string>

#include "entity.h"
#include "repository.h"

namespace ddd {
namespace domain {

class LineItem : public Entity<LineItem> {
public:
    LineItem(const std::string& item_id, const std::string& name, int price)
        : item_id_(item_id), name_(name), price_(price)
    {
    }

    bool Equals(const LineItem& other) const override
    {
        return item_id_ == other.item_id_ && name_ == other.name_ && price_ == other.price_;
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

class Order : public Entity<Order> {
public:
    static Order MakeOrder(const std::string& id, const std::vector<LineItem>& line_items)
    {
        Order result{id};
        result.line_items_ = line_items;
        for (const auto& line_item : line_items) {
            result.total_price_ += line_item.GetPrice();
        }
        return result;
    }

    Order(const std::string& id) : id_(id)
    {
    }

    ~Order() = default;

    bool Equals(const Order& other) const override
    {
        if (id_ != other.id_ || total_price_ != other.total_price_) {
            return false;
        }
        if (line_items_.size() != other.line_items_.size()) {
            return false;
        }

        for (const auto& line_item : line_items_) {
            bool matched = false;
            for (const auto& other_line_item : other.line_items_) {
                if (line_item.Equals(other_line_item)) {
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                return false;
            }
        }
        return true;
    }

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

private:
    std::string GenLineItemId() const
    {
        return std::to_string(line_items_.size());
    }

    std::string id_;
    int total_price_ = 0;
    std::vector<LineItem> line_items_;
};

}  // namespace domain
}  // namespace ddd
