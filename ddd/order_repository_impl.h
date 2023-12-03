// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include "order.h"

namespace ddd {
namespace infra {

class OrderRepositoryImpl : public domain::CollectionRepository<domain::Order> {
public:
    using EntityPtr = typename domain::CollectionRepository<domain::Order>::EntityPtr;

    OrderRepositoryImpl(DbDao& dao);

    absl::StatusOr<EntityPtr> Find(const std::string& id) override;

    EntityPtr Add(const domain::Order& entity) override;

    void Remove(const std::string& id) override;

    absl::Status Commit() override;
};

}
}
