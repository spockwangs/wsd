// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include "collection_repository_impl.h"
#include "repository.h"

namespace ddd {
namespace infra {

class LazyOrderDao {
public:
    virtual ~DbDao() = default;

    virtual absl::Status BeginTransaction() = 0;

    virtual absl::Status Commit() = 0;

    virtual absl::Status Rollback() = 0;

    virtual absl::Status SelectOrder(const std::string& id,
                                     std::shared_ptr<domain::LazyOrder>* order,
                                     std::string* cas_token) = 0;

    virtual absl::Status InsertOrder(const domain::LazyOrder& order) = 0;

    virtual absl::Status UpdateOrder(const domain::LazyOrder& entity, const std::string& cas_token) = 0;

    virtual absl::Status DeleteOrder(const std::string& id, const std::string& cas_token) = 0;

    virtual absl::Status CheckOrderCasToken(const std::string& id, const std::string& cas_token) = 0;

    virtual absl::Status SelectLineItems(const std::string& id,
                                         std::vector<std::shared_ptr<domain::LineItem>>* line_items) = 0;

    virtual absl::Status InsertLineItem(const std::string& id, const domain::LineItem& line_item) = 0;

    virtual absl::Status UpdateLineItem(const std::string& id,
                                        const domain::LineItem& line_item,
                                        const std::string& cas_token) = 0;

    virtual absl::Status DeleteLineItem(const std::string& id, const std::string& cas_token) = 0;

    virtual absl::Status CheckLineItemCasToken(const std::string& id, const std::string& cas_token) = 0;
};

class LazyOrderRepositoryImpl : public domain::LazyOrderRepository {
public:
    LazyOrderRepositoryImpl(LazyOrderDao& dao);

    ~LazyOrderRepositoryImpl() = default;

    absl::StatusOr<LazyOrderPtr> Find(const std::string& id) override;

    absl::Status FindLineItems(const std::string& id, std::vector<LineItemPtr>* line_items) override;

    LazyOrderPtr AddOrder(const LazyOrder& order) override;

    void RemoveOrder(const std::string& id) override;

    LineItemPtr AddLineItem(const LineItem& line_item) override;

    void RemoveLineItem(const std::string& order_id, const std::string& id) override;
    
    absl::Status Commit() override;

private:
    template <typename T>
    struct EntityState {
        std::unique_ptr<T> snapshot;
        std::string cas_token;
        std::shared_ptr<T> entity_ptr;
    };

    template <typename T>
    using IdMap = std::unordered_map<std::string, EntityState<T>>;

    IdMap<domain::LazyOrder> lazy_order_id_map_;
    IdMap<domain::LineItem> line_item_id_map_;
    LazyOrderDao& dao_;
};

}  // namespace infra
}  // namespace ddd
