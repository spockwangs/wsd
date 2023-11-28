// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include <mutex>

#include "absl/status/status.h"
#include "repository.h"

class Order : public Entity {
public:
    Order() = default;

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

class OrderDao {
public:
    OrderDao() = default;

    ~OrderDao() = default;
    absl::Status Get(const std::string& id, Order* order, std::string* cas_token);

    absl::Status CasPut(const Order& order, const std::string& cas_token);

    void Remove(const std::string& id);

private:
    static absl::Status FromOrderPO(const std::string& s, Order* order);

    static std::string ToOrderPO(const Order& order);

    std::mutex mu_;
    std::unordered_map<std::string, std::pair<std::string, int>> kv_;
};

class OrderRepository : public Repository<Order> {
public:
    OrderRepository() = default;

    Order* Find(const std::string& id) override;

    void Remove(const Order& order) override;

    bool Save(const Order& order) override;

    void Remove(const Order& order) override;

    bool Save(const Order& order) override;

private:
    template <typename Entity>
    struct EntityState {
        std::unique_ptr<Entity> order_ptr;
        std::string cas_token;
    };

    template <typename Entity>
    using IdMap = std::unordered_map<std::string, EntityState<Entity>>;

    IdMap<Order> id_map_;
    OrderDao dao_;
};
