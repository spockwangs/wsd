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

absl::Status LazyOrderDaoDbImpl::SelectOrder(const std::string& id, LazyOrderDto* order_dto, std::string* cas_token)
{
    auto it = order_map_.find(id);
    if (it == order_map_.end()) {
        return absl::NotFoundError("");
    }

    *order_dto = ToOrderDto(it->second);
    *cas_token = std::to_string(it->second.version);
    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::InsertOrder(const LazyOrderDto& order)
{
    LazyOrderPo order_po = FromLazyOrder(order, 0);
    if (order_map_.insert({order_po.id, order_po}).second == false) {
        return absl::AbortedError("");
    }

    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::UpdateOrder(const LazyOrderDto& order, const std::string& cas_token)
{
    auto it = order_map_.find(order.id);
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
    it->second = FromLazyOrder(order, old_version);

    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::DeleteOrder(const std::string& id, const std::string& cas_token)
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

    order_map_.erase(id);
    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::CheckOrderCasToken(const std::string& id, const std::string& cas_token)
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

absl::Status LazyOrderDaoDbImpl::SelectLineItems(
        const std::string& id, std::vector<std::pair<LineItemDto, std::string>>* line_item_cas_token_vec)
{
    for (const auto& line_item : line_item_map_) {
        if (line_item.second.order_id == id) {
            line_item_cas_token_vec->emplace_back(ToLineItemDto(line_item.second),
                                                  std::to_string(line_item.second.version));
        }
    }
    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::InsertLineItem(const LineItemDto& line_item)
{
    LineItemPo line_item_po = FromLineItem(line_item, 0);
    if (line_item_map_.insert({line_item.order_id + "/" + line_item.item_id, line_item_po}).second == false) {
        return absl::AbortedError("");
    }

    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::UpdateLineItem(const LineItemDto& line_item, const std::string& cas_token)
{
    auto it = line_item_map_.find(line_item.order_id + "/" + line_item.item_id);
    if (it == line_item_map_.end()) {
        return absl::NotFoundError("");
    }

    int old_version = 0;
    if (!absl::SimpleAtoi(cas_token, &old_version)) {
        return absl::InvalidArgumentError("");
    }
    if (old_version != it->second.version) {
        return absl::AbortedError("");
    }
    it->second = FromLineItem(line_item, old_version);

    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::DeleteLineItem(const std::string& id, const std::string& cas_token)
{
    auto it = line_item_map_.find(id);
    if (it == line_item_map_.end()) {
        return absl::NotFoundError("");
    }

    int old_version = 0;
    if (!absl::SimpleAtoi(cas_token, &old_version)) {
        return absl::InvalidArgumentError("");
    }
    if (old_version != it->second.version) {
        return absl::AbortedError("");
    }

    line_item_map_.erase(id);
    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::CheckLineItemCasToken(const std::string& id, const std::string& cas_token)
{
    auto it = line_item_map_.find(id);
    if (it == line_item_map_.end()) {
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

void LazyOrderDaoDbImpl::ResetForTesting()
{
    order_map_.clear();
    line_item_map_.clear();
}

LazyOrderDaoDbImpl::LazyOrderPo LazyOrderDaoDbImpl::FromLazyOrder(const LazyOrderDto& order, int old_version)
{
    return LazyOrderPo{.id = order.id, .total_price = order.total_price, .version = old_version + 1};
}

LazyOrderDaoDbImpl::LineItemPo LazyOrderDaoDbImpl::FromLineItem(const LineItemDto& line_item, int old_version)
{
    LineItemPo result;
    result.order_id = line_item.order_id;
    result.item_id = line_item.item_id;
    result.price = line_item.price;
    result.name = line_item.name;
    result.version = old_version + 1;
    return result;
}

LazyOrderDto LazyOrderDaoDbImpl::ToOrderDto(const LazyOrderPo& order_po)
{
    return LazyOrderDto{.id = order_po.id, .total_price = order_po.total_price};
}

LineItemDto LazyOrderDaoDbImpl::ToLineItemDto(const LineItemPo& line_item_po)
{
    return LineItemDto{.order_id = line_item_po.order_id,
                       .item_id = line_item_po.item_id,
                       .name = line_item_po.name,
                       .price = line_item_po.price};
}

}  // namespace infra
}  // namespace ddd
