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
    static Order MakeOrder(const std::string& id, const std::vector<LineItem>& line_items) {
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

    std::string GetId() const override
    {
        return id_;
    }

    int GetTotalPrice() const
    {
        return total_price_;
    }

    void AddLineItem(const std::string& name, int price) {
        line_items_.emplace_back(GenLineItemId(), name, price);
        total_price_ += price;
    }

    const std::vector<LineItem>& GetLineItems() const {
        return line_items_;
    }
    
private:
    std::string GenLineItemId() const {
        return std::to_string(line_items_.size());
    }
    
    std::string id_;
    int total_price_ = 0;
    std::vector<LineItem> line_items_;
};

}  // namespace domain
}  // namespace ddd
