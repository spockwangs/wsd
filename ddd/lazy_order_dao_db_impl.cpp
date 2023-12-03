// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "lazy_order_dao_db_impl.h"

namespace ddd {
namespace infra {

absl::Status LazyOrderDaoDbImpl::BeginTransaction()
{
    mu_.lock();
    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::Commit()
{
    mu_.unlock();
    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::Rollback()
{
    mu_.unlock();
    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::Select(const std::string& id,
                                        std::shared_ptr<domain::LazyOrder>* order,
                                        std::string* cas_token)
{
    auto it = order_map_.find(id);
    if (it == order_map_.end()) {
        return absl::NotFoundError("");
    }

    *order = std::make_shared<domain::LazyOrder>(domain::LazyOrder::MakeOrder(id, it->second.total_price));
    *cas_token = std::to_string(it->second.version);
    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::Insert(const domain::LazyOrder& order)
{
    LazyOrderDto order_dto = FromLazyOrder(order);
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

absl::Status LazyOrderDaoDbImpl::Update(const domain::LazyOrder& order, const std::string& cas_token)
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
    it->second = FromLazyOrder(order);
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

absl::Status LazyOrderDaoDbImpl::Delete(const std::string& id, const std::string& cas_token)
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

absl::Status LazyOrderDaoDbImpl::CheckCasToken(const std::string& id, const std::string& cas_token)
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

absl::Status LazyOrderDaoDbImpl::SelectLineItems(const std::string& id, std::vector<domain::LineItem>* line_items)
{
    for (const auto& line_item : line_item_map_) {
        if (line_item.second.order_id == id) {
            line_items->emplace_back(line_item.second.id, line_item.second.name, line_item.second.price);
        }
    }
    return absl::OkStatus();
}

void LazyOrderDaoDbImpl::ResetForTesting()
{
    order_map_.clear();
    line_item_map_.clear();
}

LazyOrderDaoDbImpl::OrderDto LazyOrderDaoDbImpl::FromLazyOrder(const domain::Order& order)
{
    OrderDto result;
    result.id = order.GetId();
    result.total_price = order.GetTotalPrice();
    return result;
}

LazyOrderDaoDbImpl::LineItemDto LazyOrderDaoDbImpl::FromLineItem(const std::string& order_id,
                                                                 const domain::LineItem& line_item)
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
