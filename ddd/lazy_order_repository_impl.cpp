// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "lazy_order_repository_impl.h"

namespace ddd {
namespace infra {

LazyOrderRepositoryImpl::LazyOrderRepositoryImpl(LazyOrderDao& dao) : dao_(dao)
{
}

absl::StatusOr<domain::LazyOrder*> LazyOrderRepositoryImpl::Find(const std::string& id)
{
    auto status_or_order_ptr = order_change_tracker_.Find(id);
    if (status_or_order_ptr.ok()) {
        auto* p = status_or_order_ptr.value();
        if (p == nullptr) {
            return absl::NotFoundError("");
        }
        return p;
    }

    std::string cas_token;
    LazyOrderDto order_dto;
    auto s = dao_.SelectOrder(id, &order_dto, &cas_token);
    if (!s.ok()) {
        return s;
    }
    return order_change_tracker_.RegisterClean(domain::LazyOrder::MakeOrder(*this, order_dto.id, order_dto.total_price),
                                               cas_token);
}

absl::Status LazyOrderRepositoryImpl::FindLineItems(const std::string& id, std::vector<domain::LineItem*>* line_items)
{
    std::vector<std::pair<LineItemDto, std::string>> line_item_cas_token_vec;
    auto s = dao_.SelectLineItems(id, &line_item_cas_token_vec);
    if (!s.ok()) {
        return s;
    }

    for (const auto& entry : line_item_cas_token_vec) {
        line_items->push_back(line_item_change_tracker_.RegisterClean(
                domain::LineItem{entry.first.order_id, entry.first.item_id, entry.first.name, entry.first.price},
                entry.second));
    }
    return absl::OkStatus();
}

domain::LazyOrder* LazyOrderRepositoryImpl::AddOrder(const domain::LazyOrder& order)
{
    return order_change_tracker_.RegisterUpdate(order);
}

void LazyOrderRepositoryImpl::RemoveOrder(const std::string& id)
{
    order_change_tracker_.RegisterRemove(id);
}

domain::LineItem* LazyOrderRepositoryImpl::AddLineItem(const domain::LineItem& line_item)
{
    return line_item_change_tracker_.RegisterUpdate(line_item);
}

void LazyOrderRepositoryImpl::RemoveLineItem(const std::string& line_item_id)
{
    return line_item_change_tracker_.RegisterRemove(line_item_id);
}

absl::Status LazyOrderRepositoryImpl::Commit()
{
    auto s = dao_.BeginTransaction();
    if (!s.ok()) {
        return s;
    }
    for (const auto& change : order_change_tracker_.GetChanges()) {
        switch (change.status) {
        case ChangeTracker<domain::LazyOrder>::EntityChange::Status::INSERTED:
            s = dao_.InsertOrder(ToOrderDto(*change.entity));
            break;
        case ChangeTracker<domain::LazyOrder>::EntityChange::Status::DELETED:
            s = dao_.DeleteOrder(change.entity->GetId(), change.cas_token == nullptr ? "" : *change.cas_token);
            break;
        case ChangeTracker<domain::LazyOrder>::EntityChange::Status::MODIFIED:
            s = dao_.UpdateOrder(ToOrderDto(*change.entity), *change.cas_token);
            break;
        case ChangeTracker<domain::LazyOrder>::EntityChange::Status::CLEAN:
            s = dao_.CheckOrderCasToken(change.entity->GetId(), *change.cas_token);
            break;
        }
        if (!s.ok()) {
            return s;
        }
    }

    for (const auto& change : line_item_change_tracker_.GetChanges()) {
        switch (change.status) {
        case ChangeTracker<domain::LineItem>::EntityChange::Status::INSERTED:
            s = dao_.InsertLineItem(ToLineItemDto(*change.entity));
            break;
        case ChangeTracker<domain::LineItem>::EntityChange::Status::DELETED:
            s = dao_.DeleteLineItem(change.entity->GetId(), change.cas_token == nullptr ? "" : *change.cas_token);
            break;
        case ChangeTracker<domain::LineItem>::EntityChange::Status::MODIFIED:
            s = dao_.UpdateLineItem(ToLineItemDto(*change.entity), *change.cas_token);
            break;
        case ChangeTracker<domain::LineItem>::EntityChange::Status::CLEAN:
            s = dao_.CheckLineItemCasToken(change.entity->GetId(), *change.cas_token);
            break;
        }
        if (!s.ok()) {
            return s;
        }
    }

    return dao_.Commit();
}

LazyOrderDto LazyOrderRepositoryImpl::ToOrderDto(const domain::LazyOrder& order)
{
    return LazyOrderDto{.id = order.GetId(), .total_price = order.GetTotalPrice()};
}

LineItemDto LazyOrderRepositoryImpl::ToLineItemDto(const domain::LineItem& line_item)
{
    return LineItemDto{.order_id = line_item.GetOrderId(),
                       .item_id = line_item.GetItemId(),
                       .name = line_item.GetName(),
                       .price = line_item.GetPrice()};
}

}  // namespace infra
}  // namespace ddd
