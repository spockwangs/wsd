// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "order.h"

#include <algorithm>

#include "repository.h"

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

Order Order::MakeOrder(const OrderDto& order_dto) {
{
    Order result{id};
    
    result.total_price_ = order_dto.total_price;
    for (const auto& line_item_dto : order_dto.line_items) {
        result.emplace_back(MakeLineItem(line_item_dto));
    }
    return result;
}

Order::Order(const std::string& id, LazyOrderRepository* repo) : id_(id), line_items_loaded_(true), repo_(repo)
{
}

bool Order::Equals(const Order& other) const
{
    if (id_ != other.id_ || total_price_ != other.total_price_) {
        return false;
    }

    if (line_items_loaded_ != other.line_items_loaded_) {
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
    auto s = LoadLineItemsIfNecessary();
    if (!s.ok()) {
        return s;
    }

    line_items_.emplace_back(GenLineItemId(), name, price);
    total_price_ += price;
}

absl::Status Order::RemoveLineItem(const std::string& line_item_id)
{
    auto s = LoadLineItemsIfNecessary();
    if (!s.ok()) {
        return s;
    }

    line_items_.erase(std::remove_if(line_items_.begin(), line_items_.end(),
                                     [&line_item_id](const LineItem& item) { return item.GetId() == line_item_id; }));
}

const std::vector<LineItem>& Order::GetLineItems() const
{
    auto s = LoadLineItemsIfNecessary();
    if (!s.ok()) {
        return s;
    }

    return line_items_;
}

std::string Order::GenLineItemId() const
{
    return std::to_string(line_items_.size());
}

absl::Status Order::LoadLineItemsIfNecessary()
{
    if (!line_items_loaded_) {
        auto s = repo_.FindLineItems(id_, &line_items_);
        if (!s.ok()) {
            return s;
        }
        line_items_loaded_ = true;
    }
    return absl::OkStatus();
}

}  // namespace domain
}  // namespace ddd
