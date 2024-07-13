// Copyright (c) 2023 Tencent
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#pragma once

#include <mutex>
#include <unordered_map>

#include "collection_repository_impl.h"
#include "order.h"

namespace ddd {
namespace infra {

class OrderDaoDbImpl : public DbDao<domain::Order> {
public:
    OrderDaoDbImpl() = default;

    virtual ~OrderDaoDbImpl() = default;

    absl::Status BeginTransaction() override;

    absl::Status Commit() override;

    absl::Status Rollback() override;

    absl::Status Select(const std::string& id, std::shared_ptr<domain::Order>* entity, std::string* cas_token) override;

    absl::Status Insert(const domain::Order& entity) override;

    absl::Status Update(const domain::Order& entity, const std::string& cas_token) override;

    absl::Status Delete(const std::string& id, const std::string& cas_token) override;

    absl::Status CheckCasToken(const std::string& id, const std::string& cas_token) override;

    void ResetForTesting();

private:
    struct OrderRecord {
        std::string id;
        int total_price = 0;
        int version = 0;
    };

    struct LineItemRecord {
        std::string order_id;
        std::string id;
        int price = 0;
        std::string name;
    };

    static OrderRecord FromOrder(const domain::Order& order);

    static LineItemRecord FromLineItem(const std::string& order_id, const domain::LineItem& line_item);

    std::mutex mu_;

    // Order ID => Order
    std::unordered_map<std::string, OrderRecord> order_map_;

    // Order ID/LineItem ID => LineItem
    std::unordered_map<std::string, LineItemRecord> line_item_map_;
};

}  // namespace infra
}  // namespace ddd
