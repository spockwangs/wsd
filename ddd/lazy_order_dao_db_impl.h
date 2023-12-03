// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include "lazy_order_repository_impl.h"

namespace ddd {
namespace infra {

class LazyOrderDaoDbImpl : public LazyOrderDao {
public:
    LazyOrderDaoDbImpl() = default;

    virtual ~LazyOrderDaoDbImpl() = default;

    absl::Status BeginTransaction() override;

    absl::Status Commit() override;

    absl::Status Rollback() override;

    absl::Status Select(const std::string& id,
                        std::shared_ptr<domain::LazyOrder>* entity,
                        std::string* cas_token) override;

    absl::Status Insert(const domain::LazyOrder& entity) override;

    absl::Status Update(const domain::LazyOrder& entity, const std::string& cas_token) override;

    absl::Status Delete(const std::string& id, const std::string& cas_token) override;

    absl::Status CheckCasToken(const std::string& id, const std::string& cas_token) override;

    absl::Status SelectLineItems(const std::string& id, std::vector<domain::LineItem>* line_items) override;

    void ResetForTesting();

private:
    struct LazyOrderDto {
        std::string id;
        int total_price = 0;
        int version = 0;
    };

    struct LineItemDto {
        std::string order_id;
        std::string id;
        int price = 0;
        std::string name;
    };

    static LazyOrderDto FromLazyOrder(const domain::LazyOrder& order);

    static LineItemDto FromLineItem(const std::string& order_id, const domain::LineItem& line_item);

    std::mutex mu_;

    // Order ID => LazyOrder
    std::unordered_map<std::string, LazyOrderDto> order_map_;

    // Order ID/LineItem ID => LineItem
    std::unordered_map<std::string, LineItemDto> line_item_map_;
};

}  // namespace infra
}  // namespace ddd
