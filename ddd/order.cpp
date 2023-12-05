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

LineItem LineItem::MakeLineItem(const LineItemDto& line_item_dto)
{
    return LineItem{line_item_dto.order_id, line_item_dto.item_id, line_item_dto.name, line_item_dto.price};
}

LineItem::LineItem(const std::string& order_id, const std::string& item_id, const std::string& name, int price)
    : order_id_(order_id), item_id_(item_id), name_(name), price_(price)
{
}

bool LineItem::Equals(const LineItem& other) const
{
    return order_id_ == other.order_id_ && item_id_ == other.item_id_ && name_ == other.name_ && price_ == other.price_;
}

std::string LineItem::GetId() const
{
    return order_id_ + "/" + item_id_;
}

std::string LineItem::GetOrderId() const
{
    return order_id_;
}

std::string LineItem::GetItemId() const
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

Order Order::MakeOrder(const OrderDto& order_dto)
{
    Order result{order_dto.id};

    result.total_price_ = order_dto.total_price;
    for (const auto& line_item_dto : order_dto.line_items) {
        result.line_items_.emplace_back(LineItem::MakeLineItem(line_item_dto));
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
    line_items_.emplace_back(id_, GenLineItemId(), name, price);
    total_price_ += price;
}

void Order::RemoveLineItem(const std::string& line_item_id)
{
    line_items_.erase(
            std::remove_if(line_items_.begin(), line_items_.end(), [this, &line_item_id](const LineItem& item) {
                if (item.GetId() == line_item_id) {
                    total_price_ -= item.GetPrice();
                    return true;
                }
                return false;
            }));
}

const std::vector<LineItem>& Order::GetLineItems() const
{
    return line_items_;
}

std::string Order::GenLineItemId() const
{
    return std::to_string(line_items_.size());
}

LazyOrder LazyOrder::MakeOrder(LazyOrderRepository& repo, const std::string& id, int total_price)
{
    LazyOrder order{repo, id};
    order.total_price_ = total_price;
    order.line_items_loaded_ = false;
    return order;
}

LazyOrder::LazyOrder(LazyOrderRepository& repo, const std::string& id)
    : repo_(repo), id_(id), total_price_(0), line_items_loaded_(true)
{
}

LazyOrder& LazyOrder::operator=(const LazyOrder& other)
{
    if (this == &other) {
        return *this;
    }

    id_ = other.id_;
    total_price_ = other.total_price_;
    line_items_loaded_ = other.line_items_loaded_;
    line_items_ = other.line_items_;
    return *this;
}

bool LazyOrder::Equals(const LazyOrder& other) const
{
    if (id_ != other.id_ || total_price_ != other.total_price_) {
        return false;
    }

    return true;
}

std::string LazyOrder::GetId() const
{
    return id_;
}

int LazyOrder::GetTotalPrice() const
{
    return total_price_;
}

absl::Status LazyOrder::AddLineItem(const std::string& name, int price)
{
    auto s = LoadLineItemsIfNecessary();
    if (!s.ok()) {
        return s;
    }
    line_items_.push_back(repo_.AddLineItem(LineItem{id_, GenLineItemId(), name, price}));
    total_price_ += price;
    return absl::OkStatus();
}

absl::Status LazyOrder::RemoveLineItem(const std::string& item_id)
{
    auto s = LoadLineItemsIfNecessary();
    if (!s.ok()) {
        return s;
    }
    line_items_.erase(std::remove_if(line_items_.begin(), line_items_.end(), [this, &item_id](LineItem* item) {
        if (item->GetItemId() == item_id) {
            total_price_ -= item->GetPrice();
            repo_.RemoveLineItem(item->GetId());
            return true;
        }
        return false;
    }));
    return absl::OkStatus();
}

absl::Status LazyOrder::GetLineItems(std::vector<LineItem*>* line_items) const
{
    auto s = LoadLineItemsIfNecessary();
    if (!s.ok()) {
        return s;
    }
    *line_items = line_items_;
    return absl::OkStatus();
}

std::string LazyOrder::GenLineItemId() const
{
    return std::to_string(line_items_.size());
}

absl::Status LazyOrder::LoadLineItemsIfNecessary() const
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
