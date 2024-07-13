// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include <optional>
#include <string>

#include "entity.h"
#include "repository.h"

namespace ddd {
namespace domain {

struct LineItemDto {
    std::string id;
    std::string order_id;
    std::string item_id;
    std::string name;
    int price;
};

class LineItem final : public Entity<std::string, LineItem> {
public:
    static LineItem MakeLineItem(const LineItemDto& line_item_dto);

    LineItem(const std::string& order_id, const std::string& item_id, const std::string& name, int price);

    bool Equals(const LineItem& other) const override;

    std::string GetId() const override;

    std::string GetOrderId() const;

    std::string GetItemId() const;

    const std::string& GetName() const;

    int GetPrice() const;

    // Externalize the state for persistence.
    LineItemDto ToDto() const;

private:
    std::string order_id_;
    std::string item_id_;
    std::string name_;
    int price_ = 0;
};

struct OrderDto {
    std::string id;
    int total_price;
    std::vector<LineItemDto> line_items;
};

class Order : public Entity<std::string, Order> {
public:
    static Order MakeOrder(const OrderDto& order_dto);

    explicit Order(const std::string& id);

    ~Order() = default;

    bool Equals(const Order& other) const override;

    std::string GetId() const override;

    int GetTotalPrice() const;

    void AddLineItem(const std::string& name, int price);

    void RemoveLineItem(const std::string& line_item_id);

    const std::vector<LineItem>& GetLineItems() const;

    // Externalize the state for persistence.
    OrderDto ToDto() const;

private:
    std::string GenLineItemId() const;

    std::string id_;
    int total_price_ = 0;
    std::vector<LineItem> line_items_;
};

struct LazyOrderDto {
    std::string id;
    int total_price;
};

class LazyOrder : public Entity<std::string, LazyOrder> {
public:
    static LazyOrder MakeOrder(LazyOrderRepository& repo, const std::string& id, int total_price);

    LazyOrder(LazyOrderRepository& repo, const std::string& id);

    ~LazyOrder() = default;

    LazyOrder& operator=(const LazyOrder& other);

    bool Equals(const LazyOrder& other) const override;

    std::string GetId() const override;

    int GetTotalPrice() const;

    absl::Status AddLineItem(const std::string& name, int price);

    absl::Status RemoveLineItem(const std::string& line_item_id);

    absl::Status GetLineItems(std::vector<LineItem*>* line_items) const;

    // Externalize the state for persistence.
    LazyOrderDto ToDto() const;

private:
    std::string GenLineItemId() const;

    absl::Status LoadLineItemsIfNecessary() const;

    LazyOrderRepository& repo_;
    std::string id_;
    int total_price_ = 0;
    mutable std::optional<std::vector<LineItem*>> line_items_;
};

}  // namespace domain
}  // namespace ddd
