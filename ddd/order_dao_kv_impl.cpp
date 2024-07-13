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

    domain::OrderDto order_dto;
    order_dto.id = order_po.id();
    order_dto.total_price = order_po.total_price();
    std::vector<domain::LineItem> line_items;
    for (const auto& item : order_po.line_item()) {
        order_dto.line_items.emplace_back(domain::LineItemDto{
                .order_id = order_po.id(), .item_id = item.id(), .name = item.name(), .price = item.price()});
    }
    *order_ptr = std::make_shared<domain::Order>(domain::Order::MakeOrder(order_dto));
    return absl::OkStatus();
}

std::string ToOrderPO(const domain::Order& order)
{
  domain::OrderDto dto = order.ToDto();
    infra::Order order_po;
    order_po.set_id(dto.id);
    order_po.set_total_price(dto.total_price);
    for (const auto& item : dto.line_items) {
        auto* p = order_po.add_line_item();
        p->set_id(item.id);
        p->set_name(item.name);
        p->set_price(item.price);
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
