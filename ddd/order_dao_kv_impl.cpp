// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "order_dao_kv_impl.h"

#include "ddd/order.pb.h"

namespace ddd {
namespace infra {

namespace {

absl::Status FromOrderPO(const std::string& s, std::shared_ptr<domain::Order>* order_ptr)
{
    infra::Order order_po;
    if (!order_po.ParseFromString(s)) {
        return absl::DataLossError("ParseFromString() failed");
    }

    std::vector<domain::LineItem> line_items;
    for (const auto& item : order_po.line_item()) {
        line_items.emplace_back(item.id(), item.name(), item.price());
    }
    *order_ptr = std::make_shared<domain::Order>(domain::Order::MakeOrder(order_po.id(), line_items));
    return absl::OkStatus();
}

std::string ToOrderPO(const domain::Order& order)
{
    infra::Order order_po;
    order_po.set_id(order.GetId());
    order_po.set_total_price(order.GetTotalPrice());
    for (const auto& item : order.GetLineItems()) {
        auto* p = order_po.add_line_item();
        p->set_id(item.GetId());
        p->set_name(item.GetName());
        p->set_price(item.GetPrice());
    }

    std::string result;
    order_po.SerializeToString(&result);
    return result;
}

}  // namespace

absl::Status OrderDaoKvImpl::Get(const std::string& id,
                                 std::shared_ptr<domain::Order>* order_ptr,
                                 std::string* cas_token)
{
    std::unique_lock<std::mutex> l{mu_};
    auto it = kv_.find(id);
    if (it == kv_.end()) {
        return absl::NotFoundError("");
    }

    *cas_token = std::to_string(it->second.second);
    return FromOrderPO(it->second.first, order_ptr);
}

absl::Status OrderDaoKvImpl::CasPut(const domain::Order& order, const std::string& cas_token)
{
    int version = 0;
    if (!cas_token.empty() && !absl::SimpleAtoi(cas_token, &version)) {
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

absl::Status OrderDaoKvImpl::Del(const std::string& id)
{
    std::unique_lock<std::mutex> l{mu_};
    kv_.erase(id);
    return absl::OkStatus();
}

void OrderDaoKvImpl::Reset()
{
    kv_.clear();
}

}  // namespace infra
}  // namespace ddd
