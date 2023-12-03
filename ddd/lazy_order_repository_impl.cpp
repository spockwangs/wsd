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

absl::StatusOr<LazyOrderPtr> LazyOrderRepositoryImpl::Find(const std::string& id)
{
    auto it = lazy_order_id_map_.find(id);
    if (it != lazy_order_id_map_.end()) {
        if (it->second.entity_ptr == nullptr) {
            return absl::NotFoundError("");
        }
        return it->second.entity_ptr;
    }

    std::shared_ptr<domain::LazyOrder> entity_ptr;
    std::string cas_token;
    auto s = dao_.SelectOrder(id, &entity_ptr, &cas_token);
    if (!s.ok()) {
        return s;
    }
    lazy_order_id_map_.insert(
            {id,
             EntityState<domain::LazyOrder>{.snapshot = std::make_unique<domain::LazyOrder>(*entity_ptr),
                                            .cas_token = cas_token,
                                            .entity_ptr = entity_ptr}});
    return entity_ptr;
}

absl::Status LazyOrderRepositoryImpl::FindLineItems(const std::string& id, std::vector<LineItemPtr>* line_items)
{
    std::vector < std::pair<std::shared_ptr<domain::LineItem>, std::string> line_item_ptr_cas_token_vec;
    auto s = dao_.SelectLineItems(id, &line_item_ptr_cas_token_vec);
    if (!s.ok()) {
        return s;
    }

    for (const auto& entry : line_item_ptr_cas_token_vec) {
        line_item_id_map_.insert(
                {id + "/" + entry.first->GetId(),
                 EntityState<domain::LineItem>{.snapshot = std::make_unique<domain::LineItem>(*entry.first),
                                               .cas_token = entry.second,
                                               .entity_ptr = entry.first}});
        line_items->emplace_back(entry.first);
    }
    return absl::OkStatus();
}

LazyOrderPtr LazyOrderRepositoryImpl::AddOrder(const LazyOrder& order)
{
    auto it = lazy_order_id_map_.find(order.GetId());
    if (it == lazy_order_id_map_.end()) {
        std::shared_ptr<domain::LazyOrder> entity_ptr = std::make_shared<domain::LazyOrder>(order);
        id_map_.insert({order.GetId(), EntityState{.snapshot = nullptr, .cas_token = "", .entity_ptr = entity_ptr}});
        return entity_ptr;
    }

    *it->second.entity_ptr = entity;
    return it->second.entity_ptr;
}

void LazyOrderRepositoryImpl::RemoveOrder(const std::string& id)
{
    auto it = lazy_order_id_map_.find(id);
    if (it == lazy_order_id_map_.end()) {
        lazy_order_id_map_.insert({id, EntityState<domain::LazyOrder>{}});
    } else {
        it->second.entity_ptr.reset();
    }
}

LineItemPtr LazyOrderRepositoryImpl::AddLineItem(const LineItem& line_item)
{
    const std::string unique_id = line_item.GetOrderId() + "/" + line_item.GetId();
    auto it = line_item_id_map_.find(unique_id);
    if (it == line_item_id_map_.end()) {
        std::shared_ptr<domain::LineItem> entity_ptr = std::make_shared<domain::LineItem>(order);
        line_item_id_map_.insert(
                {unique_id, EntityState{.snapshot = nullptr, .cas_token = "", .entity_ptr = entity_ptr}});
        return entity_ptr;
    }

    *it->second.entity_ptr = entity;
    return it->second.entity_ptr;
}

void LazyOrderRepositoryImpl::RemoveLineItem(const std::string& order_id, const std::string& id)
{
    const std::string unique_id = order_id + "/" + id;
    auto it = line_item_id_map_.find(unique_id);
    if (it == line_item_id_map_.end()) {
        line_item_id_map_.insert({unique_id, EntityState<domain::LineItem>{}});
    } else {
        it->second.entity_ptr.reset();
    }
}

absl::Status LazyOrderRepositoryImpl::Commit()
{
    auto s = dao_.BeginTransaction();
    if (!s.ok()) {
        return s;
    }
    for (const auto& entry : lazy_order_id_map_) {
        const auto& entity_state = entry.second;
        if (entity_state.snapshot == nullptr) {
            if (entity_state.entity_ptr == nullptr) {
                // Deleted blindly.
                s = dao_.Delete(entity_state.entity_ptr->GetId(), "");
            } else {
                // Inserted.
                s = dao_.Insert(*entity_state.entity_ptr);
            }
        } else if (entity_state.entity_ptr == nullptr) {
            // Deleted conditionally.
            s = dao_.Delete(entity_state.entity_ptr->GetId(), entity_state.cas_token);
        } else if (!entity_state.snapshot->Equals(*entity_state.entity_ptr)) {
            // Updated.
            s = dao_.Update(*entity_state.entity_ptr, entity_state.cas_token);
        } else {
            // Clean.
            s = dao_.CheckCasToken(entity_state.entity_ptr->GetId(), entity_state.cas_token);
        }
        if (!s.ok()) {
            return s;
        }
    }
    return dao_.Commit();
}

}  // namespace infra
}  // namespace ddd
