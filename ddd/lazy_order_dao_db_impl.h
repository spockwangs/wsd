// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include <mutex>

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

    absl::Status SelectOrder(const std::string& id, domain::LazyOrderDto* entity, std::string* cas_token) override;

    absl::Status InsertOrder(const domain::LazyOrder& entity) override;

    absl::Status UpdateOrder(const domain::LazyOrder& entity, const std::string& cas_token) override;

    absl::Status DeleteOrder(const std::string& id, const std::string& cas_token) override;

    absl::Status CheckOrderCasToken(const std::string& id, const std::string& cas_token) override;

    absl::Status SelectLineItems(
            const std::string& id,
            std::vector<std::pair<domain::LineItemDto, std::string>>* line_item_cas_token_vec) override;

    absl::Status InsertLineItem(const domain::LineItem& line_item) override;

    absl::Status UpdateLineItem(const domain::LineItem& line_item, const std::string& cas_token) override;

    absl::Status DeleteLineItem(const std::string& id, const std::string& cas_token) override;

    absl::Status CheckLineItemCasToken(const std::string& id, const std::string& cas_token) override;

    void ResetForTesting();

private:
    struct LazyOrderPo {
        std::string id;
        int total_price;
        int version;
    };

    struct LineItemPo {
        std::string order_id;
        std::string item_id;
        int price = 0;
        std::string name;
        int version = 0;
    };

    static domain::LazyOrderDto ToOrderDto(const LazyOrderPo& order_po);

    static domain::LineItemDto ToLineItemDto(const LineItemPo& line_item_po);

    static LazyOrderPo FromLazyOrder(const domain::LazyOrder& order);

    static LineItemPo FromLineItem(const domain::LineItem& line_item);

    std::mutex mu_;

    // Order ID => LazyOrder
    std::unordered_map<std::string, LazyOrderPo> order_map_;

    // Order ID/LineItem ID => LineItem
    std::unordered_map<std::string, LineItemPo> line_item_map_;
};

}  // namespace infra
}  // namespace ddd
