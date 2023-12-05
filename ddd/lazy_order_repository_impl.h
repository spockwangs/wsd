// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include "change_tracker.h"
#include "collection_repository_impl.h"
#include "order.h"
#include "repository.h"

namespace ddd {
namespace infra {

class LazyOrderDao {
public:
    virtual ~LazyOrderDao() = default;

    virtual absl::Status BeginTransaction() = 0;

    virtual absl::Status Commit() = 0;

    virtual absl::Status Rollback() = 0;

    virtual absl::Status SelectOrder(const std::string& id,
                                     domain::LazyOrderDto* order_dto,
                                     std::string* cas_token) = 0;

    virtual absl::Status InsertOrder(const domain::LazyOrder& order) = 0;

    virtual absl::Status UpdateOrder(const domain::LazyOrder& entity, const std::string& cas_token) = 0;

    virtual absl::Status DeleteOrder(const std::string& id, const std::string& cas_token) = 0;

    virtual absl::Status CheckOrderCasToken(const std::string& id, const std::string& cas_token) = 0;

    virtual absl::Status SelectLineItems(
            const std::string& id,
            std::vector<std::pair<domain::LineItemDto, std::string>>* line_item_cas_token_vec) = 0;

    virtual absl::Status InsertLineItem(const domain::LineItem& line_item) = 0;

    virtual absl::Status UpdateLineItem(const domain::LineItem& line_item, const std::string& cas_token) = 0;

    virtual absl::Status DeleteLineItem(const std::string& id, const std::string& cas_token) = 0;

    virtual absl::Status CheckLineItemCasToken(const std::string& id, const std::string& cas_token) = 0;
};

class LazyOrderRepositoryImpl : public domain::LazyOrderRepository {
public:
    LazyOrderRepositoryImpl(LazyOrderDao& dao);

    ~LazyOrderRepositoryImpl() = default;

    absl::StatusOr<domain::LazyOrder*> Find(const std::string& id) override;

    absl::Status FindLineItems(const std::string& id, std::vector<domain::LineItem*>* line_items) override;

    domain::LazyOrder* AddOrder(const domain::LazyOrder& order) override;

    void RemoveOrder(const std::string& id) override;

    domain::LineItem* AddLineItem(const domain::LineItem& line_item) override;

    void RemoveLineItem(const std::string& line_item_id) override;

    absl::Status Commit() override;

private:
    ChangeTracker<domain::LazyOrder> order_change_tracker_;
    ChangeTracker<domain::LineItem> line_item_change_tracker_;
    LazyOrderDao& dao_;
};

}  // namespace infra
}  // namespace ddd
