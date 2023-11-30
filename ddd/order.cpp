// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "order.h"

namespace ddd {
namespace domain {

LineItem::LineItem(const std::string& item_id, const std::string& name, int price)
    : item_id_(item_id), name_(name), price_(price)
{
}

bool LineItem::Equals(const LineItem& other) const 
{
    return item_id_ == other.item_id_ && name_ == other.name_ && price_ == other.price_;
}

std::string LineItem::GetId() const 
{
    return item_id_;
}

const std::string& LineItem::GetName() const
{
    return name_;
}

int LineItem::GetPrice() const
{
    return price_;
}

Order Order::MakeOrder(const std::string& id, const std::vector<LineItem>& line_items)
{
    Order result{id};
    result.line_items_ = line_items;
    for (const auto& line_item : line_items) {
        result.total_price_ += line_item.GetPrice();
    }
    return result;
}

Order::Order(const std::string& id) : id_(id)
{
}

bool Order::Equals(const Order& other) const 
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

std::string Order::GetId() const
{
    return id_;
}

int Order::GetTotalPrice() const
{
    return total_price_;
}

void Order::AddLineItem(const std::string& name, int price)
{
    line_items_.emplace_back(GenLineItemId(), name, price);
    total_price_ += price;
}

const std::vector<LineItem>& Order::GetLineItems() const
{
    return line_items_;
}

std::string Order::GenLineItemId() const
{
    return std::to_string(line_items_.size());
}
}  // namespace domain
}  // namespace ddd
