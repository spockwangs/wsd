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

absl::StatusOr<LazyOrderRepositoryImpl::LazyOrderPtr> LazyOrderRepositoryImpl::Find(const std::string& id)
{
    auto status_or_order_ptr = order_change_tracker_.Find(id);
    if (status_or_order_ptr.ok()) {
        if (status_or_order_ptr.value().expired()) {
            return absl::NotFoundError("");
        }
        return status_or_order_ptr;
    }

    std::string cas_token;
    domain::LazyOrderDto order_dto;
    auto s = dao_.SelectOrder(id, &order_dto, &cas_token);
    if (!s.ok()) {
        return s;
    }
    return order_change_tracker_.RegisterClean(domain::LazyOrder::MakeOrder(*this, order_dto), cas_token);
}

absl::Status LazyOrderRepositoryImpl::FindLineItems(const std::string& id, std::vector<LineItemPtr>* line_items)
{
    std::vector<std::pair<domain::LineItemDto, std::string>> line_item_cas_token_vec;
    auto s = dao_.SelectLineItems(id, &line_item_cas_token_vec);
    if (!s.ok()) {
        return s;
    }

    for (const auto& entry : line_item_cas_token_vec) {
        line_items->push_back(
                line_item_change_tracker_.RegisterClean(domain::LineItem::MakeLineItem(entry.first), entry.second));
    }
    return absl::OkStatus();
}

LazyOrderRepositoryImpl::LazyOrderPtr LazyOrderRepositoryImpl::AddOrder(const domain::LazyOrder& order)
{
    return order_change_tracker_.RegisterUpdate(order);
}

void LazyOrderRepositoryImpl::RemoveOrder(const std::string& id)
{
    order_change_tracker_.RegisterRemove(id);
}

LazyOrderRepositoryImpl::LineItemPtr LazyOrderRepositoryImpl::AddLineItem(const domain::LineItem& line_item)
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
            s = dao_.InsertOrder(*change.entity);
            break;
        case ChangeTracker<domain::LazyOrder>::EntityChange::Status::DELETED:
            s = dao_.DeleteOrder(change.entity->GetId(), change.cas_token == nullptr ? "" : *change.cas_token);
            break;
        case ChangeTracker<domain::LazyOrder>::EntityChange::Status::MODIFIED:
            s = dao_.UpdateOrder(*change.entity, *change.cas_token);
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
            s = dao_.InsertLineItem(*change.entity);
            break;
        case ChangeTracker<domain::LineItem>::EntityChange::Status::DELETED:
            s = dao_.DeleteLineItem(change.entity->GetId(), change.cas_token == nullptr ? "" : *change.cas_token);
            break;
        case ChangeTracker<domain::LineItem>::EntityChange::Status::MODIFIED:
            s = dao_.UpdateLineItem(*change.entity, *change.cas_token);
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

}  // namespace infra
}  // namespace ddd
