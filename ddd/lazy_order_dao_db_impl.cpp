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

absl::Status LazyOrderDaoDbImpl::SelectOrder(const std::string& id,
                                             domain::LazyOrderDto* order_dto,
                                             std::string* cas_token)
{
    auto it = order_map_.find(id);
    if (it == order_map_.end()) {
        return absl::NotFoundError("");
    }

    *order_dto = ToLazyOrderDto(it->second);
    *cas_token = std::to_string(it->second.version);
    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::InsertOrder(const domain::LazyOrder& order)
{
    LazyOrderPo order_po = FromLazyOrder(order, 0);
    if (order_map_.insert({order_po.id, order_po}).second == false) {
        return absl::AbortedError("");
    }

    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::UpdateOrder(const domain::LazyOrder& order, const std::string& cas_token)
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
        const std::string& id, std::vector<std::pair<domain::LineItemDto, std::string>>* line_item_cas_token_vec)
{
    for (const auto& line_item : line_item_map_) {
        if (line_item.second.order_id == id) {
            line_item_cas_token_vec->emplace_back(ToLineItemDto(line_item.second),
                                                  std::to_string(line_item.second.version));
        }
    }
    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::InsertLineItem(const domain::LineItem& line_item)
{
    LineItemPo line_item_po = FromLineItem(line_item, 0);
    if (line_item_map_.insert({line_item.GetId(), line_item_po}).second == false) {
        return absl::AbortedError("");
    }

    return absl::OkStatus();
}

absl::Status LazyOrderDaoDbImpl::UpdateLineItem(const domain::LineItem& line_item, const std::string& cas_token)
{
    auto it = line_item_map_.find(line_item.GetId());
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

domain::LazyOrderDto LazyOrderDaoDbImpl::ToLazyOrderDto(const LazyOrderPo& order_po)
{
    domain::LazyOrderDto result;
    result.id = order_po.id;
    result.total_price = order_po.total_price;
    return result;
}

domain::LineItemDto LazyOrderDaoDbImpl::ToLineItemDto(const LineItemPo& line_item_po)
{
    domain::LineItemDto result;
    result.id = line_item_po.order_id + "/" + line_item_po.item_id;
    result.order_id = line_item_po.order_id;
    result.item_id = line_item_po.item_id;
    result.name = line_item_po.name;
    result.price = line_item_po.price;
    return result;
}

LazyOrderDaoDbImpl::LazyOrderPo LazyOrderDaoDbImpl::FromLazyOrder(const domain::LazyOrder& order, int old_version)
{
    domain::LazyOrderDto dto = order.ToDto();
    return LazyOrderPo{.id = dto.id, .total_price = dto.total_price, .version = old_version + 1};
}

LazyOrderDaoDbImpl::LineItemPo LazyOrderDaoDbImpl::FromLineItem(const domain::LineItem& line_item, int old_version)
{
    domain::LineItemDto dto = line_item.ToDto();
    LineItemPo result;
    result.order_id = dto.order_id;
    result.item_id = dto.item_id;
    result.price = dto.price;
    result.name = dto.name;
    result.version = old_version + 1;
    return result;
}

}  // namespace infra
}  // namespace ddd
