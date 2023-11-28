#pragma once

#include <string>
#include <type_traits>

#include "absl/status/status.h"

namespace ddd {

class Entity {
public:
    Entity() = default;

    virtual ~Entity() = default;

    Entity(const Entity&) = delete;
    void operator=(const Entity&) = delete;

    virtual std::string GetId() const = 0;
};

template <typename T, std::enable_if_t<std::is_base_of<Entity, T>::value, bool> = true>
class Repository {
public:
    Repository() = default;

    virtual ~Repository() = default;

    virtual absl::Status Find(const std::string& id, T** entity_pp) = 0;

    virtual absl::Status Remove(T&& entity) = 0;

    virtual absl::Status Save(T&& entity) = 0;
};

}  // namespace ddd
