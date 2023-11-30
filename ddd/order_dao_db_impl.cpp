// Copyright (c) 2023 Tencent
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#include "order_dao_db_impl.h"

namespace ddd {
namespace infra {

absl::Status OrderDaoDbImpl::BeginTransaction()
{
    mu_.lock();
    return absl::OkStatus();
}

absl::Status OrderDaoDbImpl::Commit()
{
    mu_.unlock();
    return absl::OkStatus();
}

absl::Status OrderDaoDbImpl::Rollback()
{
    mu_.unlock();
    return absl::OkStatus();
}

absl::Status OrderDaoDbImpl::Select(const std::string& id,
                                    std::shared_ptr<domain::Order>* order,
                                    std::string* cas_token)
{
    auto it = order_map_.find(id);
    if (it == order_map_.end()) {
        return absl::NotFoundError("");
    }

    std::vector<domain::LineItem> line_items;
    for (const auto& line_item : line_item_map_) {
        if (line_item.second.order_id == id) {
            line_items.emplace_back(line_item.second.id, line_item.second.name, line_item.second.price);
        }
    }
    *order = std::make_shared<domain::Order>(domain::Order::MakeOrder(id, line_items));
    *cas_token = std::to_string(it->second.version);
    return absl::OkStatus();
}

absl::Status OrderDaoDbImpl::Insert(const domain::Order& order)
{
    OrderDto order_dto = FromOrder(order);
    ++order_dto.version;
    if (order_map_.insert({order_dto.id, order_dto}).second == false) {
        return absl::AbortedError("");
    }

    for (const auto& line_item : order.GetLineItems()) {
        LineItemDto line_item_dto = FromLineItem(order.GetId(), line_item);
        if (line_item_map_.insert({line_item_dto.order_id + "/" + line_item_dto.id, line_item_dto}).second == false) {
            return absl::AbortedError("");
        }
    }
    return absl::OkStatus();
}

absl::Status OrderDaoDbImpl::Update(const domain::Order& order, const std::string& cas_token)
{
    auto it = order_map_.find(order.GetId());
    if (it == order_map_.end()) {
        return absl::NotFoundError("");
    }

    int old_version = 0;
    if (!absl::SimpleAtoi(cas_token, &old_version)) {
        return absl::InvalidArgumentError("");
    }
    if (old_version != it->second.version) {
        return absl::AbortedError("");
    }
    it->second = FromOrder(order);
    it->second.version = old_version + 1;

    // Delete all old line items.
    for (auto it = line_item_map_.begin(); it != line_item_map_.end();) {
        if (it->second.order_id == order.GetId()) {
            it = line_item_map_.erase(it);
        } else {
            ++it;
        }
    }

    for (const auto& line_item : order.GetLineItems()) {
        line_item_map_[order.GetId() + "/" + line_item.GetId()] = FromLineItem(order.GetId(), line_item);
    }
    return absl::OkStatus();
}

absl::Status OrderDaoDbImpl::Delete(const std::string& id, const std::string& cas_token)
{
    auto it = order_map_.find(id);
    if (it == order_map_.end()) {
        return absl::NotFoundError("");
    }

    int old_version = 0;
    if (!absl::SimpleAtoi(cas_token, &old_version)) {
        return absl::InvalidArgumentError("");
    }
    if (old_version != it->second.version) {
        return absl::AbortedError("");
    }

    for (auto it = line_item_map_.begin(); it != line_item_map_.end();) {
        if (it->second.order_id == id) {
            it = line_item_map_.erase(it);
        } else {
            ++it;
        }
    }

    order_map_.erase(id);
    return absl::OkStatus();
}

absl::Status OrderDaoDbImpl::CheckCasToken(const std::string& id, const std::string& cas_token)
{
    auto it = order_map_.find(id);
    if (it == order_map_.end()) {
        return absl::AbortedError("");
    }
    int version = 0;
    if (!absl::SimpleAtoi(cas_token, &version)) {
        return absl::InvalidArgumentError("");
    }
    if (version != it->second.version) {
        return absl::AbortedError("");
    }
    return absl::OkStatus();
}

void OrderDaoDbImpl::ResetForTesting()
{
    order_map_.clear();
    line_item_map_.clear();
}

OrderDaoDbImpl::OrderDto OrderDaoDbImpl::FromOrder(const domain::Order& order)
{
    OrderDto result;
    result.id = order.GetId();
    result.total_price = order.GetTotalPrice();
    return result;
}

OrderDaoDbImpl::LineItemDto OrderDaoDbImpl::FromLineItem(const std::string& order_id, const domain::LineItem& line_item)
{
    LineItemDto result;
    result.order_id = order_id;
    result.id = line_item.GetId();
    result.price = line_item.GetPrice();
    result.name = line_item.GetName();
    return result;
}

}  // namespace infra
}  // namespace ddd
