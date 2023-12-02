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

class LineItem final : public Entity<LineItem> {
public:
    LineItem(const std::string& item_id, const std::string& name, int price);

    bool Equals(const LineItem& other) const override;

    std::string GetId() const override;

    const std::string& GetName() const;

    int GetPrice() const;

private:
    std::string item_id_;
    std::string name_;
    int price_ = 0;
};

class Order : public Entity<Order> {
public:
    static Order MakeOrder(const std::string& id, const std::vector<LineItem>& line_items);

    Order(const std::string& id);

    ~Order() = default;

    bool Equals(const Order& other) const override;

    std::string GetId() const override;

    int GetTotalPrice() const;

    void AddLineItem(const std::string& name, int price);

    const std::vector<LineItem>& GetLineItems() const;

private:
    std::string GenLineItemId() const;

    std::string id_;
    int total_price_ = 0;
    std::vector<LineItem> line_items_;
};

}  // namespace domain
}  // namespace ddd
