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

template <typename Entity>
class Dao {
public:
    virtual ~Dao() = default;

    virtual absl::StatusOr<std::unique_ptr<Entity>> Get(const std::string& id, std::string* cas_token) = 0;

    virtual absl::Status CasPut(const Entity& order, const std::string& cas_token) = 0;

    virtual absl::Status Del(const std::string& id) = 0;
};

template <typename Entity>
class RepositoryImpl : public Repository<Entity> {
public:
    RepositoryImpl(Dao<Entity>& dao) : dao_(dao)
    {
    }

    ~RepositoryImpl() override = default;

    absl::StatusOr<Entity*> Find(const std::string& id) override
    {
        auto it = id_map_.find(id);
        if (it != id_map_.end()) {
            return it->second.entity_ptr.get();
        }

        std::string cas_token;
        auto status_or_entity_ptr = dao_.Get(id, &cas_token);
        if (!status_or_entity_ptr.ok()) {
          return status_or_entity_ptr.status();
        }
        it = id_map_.emplace(id, EntityState{.entity_ptr = std::move(*status_or_entity_ptr), .cas_token = cas_token})
                     .first;
        return it->second.entity_ptr.get();
    }

    absl::Status Remove(const std::string& id) override
    {
        id_map_.erase(id);
        return dao_.Del(id);
    }

    absl::Status Save(const Entity& entity) override
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
        std::unique_ptr<Entity> entity_ptr;
        std::string cas_token;
    };

    using IdMap = std::unordered_map<std::string, EntityState>;

    IdMap id_map_;
    Dao<Entity>& dao_;
};

}  // namespace ddd
