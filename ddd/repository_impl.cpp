// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "repository_impl.h"

#include "absl/status/status.h"
#include "ddd/order.pb.h"
#include "repository.h"
#include <iostream>

using namespace std;

absl::Status OrderDao::Get(const std::string& id, Order* order, std::string* cas_token)
{
    std::unique_lock<std::mutex> l{mu_};
    auto it = kv_.find(id);
    if (it == kv_.end()) {
        return absl::NotFoundError("");
    }
    auto status = FromOrderPO(it->second.first, order);
    if (!status.ok()) {
        return status;
    }
    *cas_token = std::to_string(it->second.second);
    return absl::OkStatus();
}

absl::Status OrderDao::CasPut(const Order& order, const std::string& cas_token)
{
    int version = 0;
    if (!absl::SimpleAtoi(cas_token, &version)) {
        return absl::InvalidArgumentError("invalid cas_token");
    }

    int exp_version = 0;
    std::unique_lock<std::mutex> l{mu_};
    auto it = kv_.find(order.GetId());
    if (it == kv_.end()) {
        exp_version = 0;
    } else {
        exp_version = it->second.second;
    }
    if (version != exp_version) {
        return absl::AbortedError("");
    }
    kv_[order.GetId()] = std::make_pair(ToOrderPO(order), ++version);
    return absl::OkStatus();
}

void OrderDao::Remove(const std::string& id)
{
    std::unique_lock<std::mutex> l{mu_};
    kv_.erase(id);
}

absl::Status OrderDao::FromOrderPO(const std::string& s, Order* order)
{
    ddd::Order order_po;
    if (!order_po.ParseFromString(s)) {
        return absl::DataLossError("ParseFromString() failed");
    }
    order->SetPrice(order_po.price());
    return absl::OkStatus();
}

std::string OrderDao::ToOrderPO(const Order& order)
{
    ddd::Order order_po;
    order_po.set_price(order.GetPrice());
    std::string result;
    order_po.SerializeToString(&result);
    return result;
}

Order* OrderRepository::Find(const std::string& id)
{
    auto it = id_map_.find(id);
    if (it != id_map_.end()) {
        return it->second.order_ptr.get();
    }

    std::string cas_token;
    std::unique_ptr<Order> order_ptr{new Order{}};
    auto status = dao_.Get(id, order_ptr.get(), &cas_token);
    if (!status.ok()) {
        return nullptr;
    }
    it = id_map_.emplace(id, EntityState<Order>{.order_ptr = std::move(order_ptr), .cas_token = cas_token}).first;
    return it->second.order_ptr.get();
}

void OrderRepository::Remove(const Order& order)
{
    dao_.Remove(order.GetId());
    id_map_.erase(order.GetId());
}

bool OrderRepository::Save(const Order& order)
{
    std::string cas_token;
    auto it = id_map_.find(order.GetId());
    if (it != id_map_.end()) {
        cas_token = it->second.cas_token;
        id_map_.erase(order.GetId());
    }
    // For new object, use empty cas token.
    auto status = dao_.CasPut(order, cas_token);
    return status.ok();
}
