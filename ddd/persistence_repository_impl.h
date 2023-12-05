// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "repository.h"

namespace ddd {
namespace infra {

template <typename Entity>
class Dao {
public:
    virtual ~Dao() = default;

    virtual absl::Status Get(const std::string& id, std::shared_ptr<Entity>* entity, std::string* cas_token) = 0;

    virtual absl::Status CasPut(const Entity& order, const std::string& cas_token) = 0;

    virtual absl::Status Del(const std::string& id) = 0;
};

template <typename ID, typename T>
class PersistenceRepositoryImpl : public domain::PersistenceRepository<ID, T> {
public:
    using EntityPtr = typename domain::PersistenceRepository<ID, T>::EntityPtr;

    PersistenceRepositoryImpl(Dao<T>& dao) : dao_(dao)
    {
    }

    ~PersistenceRepositoryImpl() override = default;

    absl::StatusOr<EntityPtr> Find(const std::string& id) override
    {
        auto it = id_map_.find(id);
        if (it != id_map_.end()) {
            return it->second.entity_ptr;
        }

        std::string cas_token;
        std::shared_ptr<T> entity_ptr;
        auto status = dao_.Get(id, &entity_ptr, &cas_token);
        if (!status.ok()) {
            return status;
        }
        id_map_.emplace(id, EntityState{.entity_ptr = entity_ptr, .cas_token = cas_token});
        return entity_ptr;
    }

    absl::Status Remove(const std::string& id) override
    {
        id_map_.erase(id);
        return dao_.Del(id);
    }

    absl::Status Save(const T& entity) override
    {
        std::string cas_token;
        auto it = id_map_.find(entity.GetId());
        if (it != id_map_.end()) {
            cas_token = it->second.cas_token;
            id_map_.erase(entity.GetId());
        }
        // For new object, use empty cas token.
        return dao_.CasPut(entity, cas_token);
    }

private:
    struct EntityState {
        std::shared_ptr<T> entity_ptr;
        std::string cas_token;
    };

    using IdMap = std::unordered_map<std::string, EntityState>;

    IdMap id_map_;
    Dao<T>& dao_;
};

}  // namespace infra
}  // namespace ddd
