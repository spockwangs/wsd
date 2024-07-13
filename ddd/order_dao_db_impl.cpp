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

    domain::OrderDto order_dto;
    order_dto.id = id;
    order_dto.total_price = it->second.total_price;
    for (const auto& line_item : line_item_map_) {
        if (line_item.second.order_id == id) {
            order_dto.line_items.emplace_back(domain::LineItemDto{.order_id = line_item.second.order_id,
                                                                  .item_id = line_item.second.id,
                                                                  .name = line_item.second.name,
                                                                  .price = line_item.second.price});
        }
    }
    *order = std::make_shared<domain::Order>(domain::Order::MakeOrder(order_dto));
    *cas_token = std::to_string(it->second.version);
    return absl::OkStatus();
}

absl::Status OrderDaoDbImpl::Insert(const domain::Order& order)
{
    OrderRecord order_dto = FromOrder(order);
    ++order_dto.version;
    if (order_map_.insert({order_dto.id, order_dto}).second == false) {
        return absl::AbortedError("");
    }

    for (const auto& line_item : order.GetLineItems()) {
        LineItemRecord line_item_dto = FromLineItem(order.GetId(), line_item);
        if (line_item_map_.insert({line_item.GetId(), line_item_dto}).second == false) {
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
        line_item_map_[line_item.GetId()] = FromLineItem(order.GetId(), line_item);
    }
    return absl::OkStatus();
}

absl::Status OrderDaoDbImpl::Delete(const std::string& id, const std::string& cas_token)
{
    auto it = order_map_.find(id);
    if (it == order_map_.end()) {
        return absl::NotFoundError("");
    }

    // If `cas_token` is empty, delete blindly without checking version.
    if (!cas_token.empty()) {
        int old_version = 0;
        if (!absl::SimpleAtoi(cas_token, &old_version)) {
            return absl::InvalidArgumentError("");
        }
        if (old_version != it->second.version) {
            return absl::AbortedError("");
        }
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

OrderDaoDbImpl::OrderRecord OrderDaoDbImpl::FromOrder(const domain::Order& order)
{
    domain::OrderDto dto = order.ToDto();
    OrderRecord result;
    result.id = dto.id;
    result.total_price = dto.total_price;
    return result;
}

OrderDaoDbImpl::LineItemRecord OrderDaoDbImpl::FromLineItem(const std::string& order_id,
                                                            const domain::LineItem& line_item)
{
    domain::LineItemDto dto = line_item.ToDto();
    LineItemRecord result;
    result.order_id = order_id;
    result.id = dto.id;
    result.price = dto.price;
    result.name = dto.name;
    return result;
}

}  // namespace infra
}  // namespace ddd
