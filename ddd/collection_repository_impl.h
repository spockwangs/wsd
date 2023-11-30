// Copyright (c) 2023 Tencent
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#pragma once

#include <unordered_map>

#include "repository.h"

namespace ddd {
namespace infra {

template <typename Entity>
class DbDao {
public:
    virtual ~DbDao() = default;

    virtual absl::Status BeginTransaction() = 0;

    virtual absl::Status Commit() = 0;

    virtual absl::Status Rollback() = 0;

    virtual absl::Status Select(const std::string& id, std::shared_ptr<Entity>* entity, std::string* cas_token) = 0;

    virtual absl::Status Insert(const Entity& entity) = 0;

    virtual absl::Status Update(const Entity& entity, const std::string& cas_token) = 0;

    virtual absl::Status Delete(const std::string& id, const std::string& cas_token) = 0;

    virtual absl::Status CheckCasToken(const std::string& id, const std::string& cas_token) = 0;
};

template <typename T>
class CollectionRepositoryImpl : public domain::CollectionRepository<T> {
public:
    using EntityPtr = typename domain::CollectionRepository<T>::EntityPtr;

    CollectionRepositoryImpl(DbDao<T>& dao) : dao_(dao)
    {
    }

    absl::StatusOr<EntityPtr> Find(const std::string& id) override
    {
        auto it = id_map_.find(id);
        if (it != id_map_.end()) {
            if (it->second.status == Status::DELETED) {
                return absl::NotFoundError("");
            } else {
                return it->second.entity_ptr;
            }
        }

        std::shared_ptr<T> entity_ptr;
        std::string cas_token;
        auto s = dao_.Select(id, &entity_ptr, &cas_token);
        if (!s.ok()) {
            return s;
        }
        id_map_.insert({id, EntityState{.entity_ptr = entity_ptr, .status = Status::CLEAN, .cas_token = cas_token}});
        return entity_ptr;
    }

    EntityPtr Add(const T& entity) override
    {
        auto it = id_map_.find(entity.GetId());
        if (it == id_map_.end()) {
            std::shared_ptr<T> entity_ptr = std::make_shared<T>(entity);
            id_map_.insert({entity.GetId(), EntityState{.entity_ptr = entity_ptr, .status = Status::NEW}});
            return entity_ptr;
        }

        if (it->second.status == Status::CLEAN || it->second.status == Status::DELETED) {
            it->second.status = Status::MODIFIED;
        }
        it->second.entity_ptr = std::make_shared<T>(entity);
        return it->second.entity_ptr;
    }

    void Remove(const std::string& id) override
    {
        auto it = id_map_.find(id);
        if (it == id_map_.end()) {
          EntityState entity_state{
            .entity_ptr = nullptr,
            .status = Status::DELETED,
            .cas_token = ""
          };
          id_map_.insert({id, entity_state});
        } else {
            it->second.status = Status::DELETED;
        }
    }

    absl::Status Commit() override
    {
        auto s = dao_.BeginTransaction();
        if (!s.ok()) {
            return s;
        }
        for (const auto& entry : id_map_) {
          const auto& entity_state = entry.second;
            switch (entity_state.status) {
            case Status::CLEAN:
                s = dao_.CheckCasToken(entity_state.entity_ptr->GetId(), entity_state.cas_token);
                break;
            case Status::MODIFIED:
                s = dao_.Update(*entity_state.entity_ptr, entity_state.cas_token);
                break;
            case Status::NEW:
                s = dao_.Insert(*entity_state.entity_ptr);
                break;
            case Status::DELETED:
                s = dao_.Delete(entity_state.entity_ptr->GetId(), entity_state.cas_token);
                break;
            }
            if (!s.ok()) {
              return s;
            }
        }
        return dao_.Commit();
    }

private:
    enum class Status { CLEAN, MODIFIED, NEW, DELETED };

    struct EntityState {
        std::shared_ptr<T> entity_ptr;
        Status status;
        std::string cas_token;
    };

    using IdMap = std::unordered_map<std::string, EntityState>;

    IdMap id_map_;
    DbDao<T>& dao_;
};

}  // namespace infra
}  // namespace ddd
