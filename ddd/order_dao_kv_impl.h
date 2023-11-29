// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include "persistence_repository_impl.h"
#include "order.h"

namespace ddd {
namespace infra {

class OrderDaoKvImpl : public Dao<domain::Order> {
public:
    OrderDaoKvImpl() = default;

    ~OrderDaoKvImpl() override = default;

    absl::Status Get(const std::string& id, std::shared_ptr<domain::Order>* order_ptr, std::string* cas_token) override;

    absl::Status CasPut(const domain::Order& order, const std::string& cas_token) override;

    absl::Status Del(const std::string& id) override;

    void Reset();

private:
    std::mutex mu_;
    std::unordered_map<std::string, std::pair<std::string, int>> kv_;
};

}  // namespace infra
}  // namespace ddd
