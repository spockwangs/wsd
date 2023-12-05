// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace ddd {
namespace infra {

template <typename T>
class ChangeTracker final {
public:
    ChangeTracker() = default;

    // Returns not found error if not tracked, nullptr if deleted, or the tracked entity.
    absl::StatusOr<T*> Find(const std::string& id)
    {
        auto it = id_map_.find(id);
        if (it != id_map_.end()) {
            return it->second.entity_ptr.get();
        }

        return absl::NotFoundError("");
    }

    T* RegisterClean(const T& entity, const std::string& cas_token)
    {
        auto it = id_map_.insert({entity.GetId(),
                                  EntityState{.snapshot = std::make_unique<T>(entity),
                                              .cas_token = cas_token,
                                              .entity_ptr = std::make_unique<T>(entity)}})
                          .first;
        return it->second.entity_ptr.get();
    }

    T* RegisterUpdate(const T& entity)
    {
        auto it = id_map_.find(entity.GetId());
        if (it == id_map_.end()) {
            std::unique_ptr<T> entity_ptr = std::make_unique<T>(entity);
            it = id_map_.insert({entity.GetId(),
                                 EntityState{
                                         .snapshot = nullptr, .cas_token = "", .entity_ptr = std::move(entity_ptr)}})
                         .first;
        } else if (it->second.entity_ptr) {
            *it->second.entity_ptr = entity;
        } else {
            it->second.entity_ptr = std::make_unique<T>(entity);
        }
        return it->second.entity_ptr.get();
    }

    void RegisterRemove(const std::string& id)
    {
        auto it = id_map_.find(id);
        if (it == id_map_.end()) {
            id_map_.insert({id, EntityState{}});
        } else {
            it->second.entity_ptr.reset();
        }
    }

    struct EntityChange {
        const T* entity;
        const std::string* cas_token;
        enum class Status { INSERTED, DELETED, MODIFIED, CLEAN };
        Status status;
    };

    std::vector<EntityChange> GetChanges() const
    {
        std::vector<EntityChange> result;
        for (const auto& entry : id_map_) {
            const auto& entity_state = entry.second;
            if (entity_state.snapshot == nullptr) {
                if (entity_state.entity_ptr == nullptr) {
                    // Deleted blindly.
                    result.push_back(EntityChange{.entity = entity_state.entity_ptr.get(),
                                                  .cas_token = nullptr,
                                                  .status = EntityChange::Status::DELETED});
                } else {
                    // Inserted.
                    result.push_back(EntityChange{.entity = entity_state.entity_ptr.get(),
                                                  .cas_token = nullptr,
                                                  .status = EntityChange::Status::INSERTED});
                }
            } else if (entity_state.entity_ptr == nullptr) {
                // Deleted conditionally.
                result.push_back(EntityChange{.entity = entity_state.entity_ptr.get(),
                                              .cas_token = &entity_state.cas_token,
                                              .status = EntityChange::Status::DELETED});
            } else if (!entity_state.snapshot->Equals(*entity_state.entity_ptr)) {
                // Updated.
                result.push_back(EntityChange{.entity = entity_state.entity_ptr.get(),
                                              .cas_token = &entity_state.cas_token,
                                              .status = EntityChange::Status::MODIFIED});
            } else {
                // Clean.
                result.push_back(EntityChange{.entity = entity_state.entity_ptr.get(),
                                              .cas_token = &entity_state.cas_token,
                                              .status = EntityChange::Status::CLEAN});
            }
        }
        return result;
    }

private:
    struct EntityState {
        std::unique_ptr<T> snapshot;
        std::string cas_token;
        std::unique_ptr<T> entity_ptr;
    };

    using IdMap = std::unordered_map<std::string, EntityState>;

    IdMap id_map_;
};
}  // namespace infra
}  // namespace ddd
