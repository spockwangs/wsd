// Copyright (c) 2023
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

namespace ddd {
namespace infra {

template <typename T>
class UnitOfWork final {
public:
    using EntityPtr = std::weak_ptr<T>;

    UnitOfWork() = default;

    EntityPtr Find(const std::string& id)
    {
        auto it = id_map_.find(id);
        if (it != id_map_.end() && it->second.entity_ptr != nullptr) {
            return it->second.entity_ptr;
        }
        return EntityPtr{};
    }

    void RegisterClean(const T& entity)
    {
        id_map_.insert({entity.GetId(),
                        EntityState{.snapshot = std::make_unique<T>(entity),
                                    .cas_token = cas_token,
                                    .entity_ptr = std::make_shared<T>(entity)}});
    }

    EntityPtr RegisterUpdate(const T& entity)
    {
        auto it = id_map_.find(entity.GetId());
        if (it == id_map_.end()) {
            std::shared_ptr<T> entity_ptr = std::make_shared<T>(entity);
            id_map_.insert(
                    {entity.GetId(), EntityState{.snapshot = nullptr, .cas_token = "", .entity_ptr = entity_ptr}});
            return entity_ptr;
        }

        *it->second.entity_ptr = entity;
        return it->second.entity_ptr;
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

    std::vector<EntityChange> GetChanges() const {
        std::vector<EntityChange> result;
        for (const auto& entry : id_map_) {
            const auto& entity_state = entry.second;
            if (entity_state.snapshot == nullptr) {
                if (entity_state.entity_ptr == nullptr) {
                    // Deleted blindly.
                    result.push_back(EntityChange{
                                         .entity = entity_state.entity_ptr.get(),
                                             .cas_token = nullptr,
                                         .status = Status::DELETED});
                } else {
                    // Inserted.
                    result.push_back(EntityChange{
                                         .entity = entity_state.entity_ptr.get(),
                                             .cas_token = nullptr,
                                         .status = Status::INSERTED});
                }
            } else if (entity_state.entity_ptr == nullptr) {
                // Deleted conditionally.
                result.push_back(EntityChange{
                                     .entity = entity_state.entity_ptr.get(),
                                     .cas_token = &entity_state.cas_token,
                                     .status = Status::DELETED});
            } else if (!entity_state.snapshot->Equals(*entity_state.entity_ptr)) {
                // Updated.
                result.push_back(EntityChange{
                                     .entity = entity_state.entity_ptr.get(),
                                         .cas_token = &entity_state.cas_token,
                                     .status = Status::MODIFIED});
            } else {
                // Clean.
                result.push_back(EntityChange{
                                     .entity = entity_state.entity_ptr.get(),
                                         .cas_token = &entity_state.cas_token,
                                     .status = Status::CLEAN});
            }
        }
        return result;
    }
    
private:
    struct EntityState {
        std::unique_ptr<T> snapshot;
        std::string cas_token;
        std::shared_ptr<T> entity_ptr;
    };

    using IdMap = std::unordered_map<std::string, EntityState>;

    IdMap id_map_;
};
}  // namespace infra
}  // namespace ddd
