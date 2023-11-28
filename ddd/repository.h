#pragma once

#include <string>
#include <type_traits>

#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace ddd {

class Entity {
public:
    Entity() = default;

    virtual ~Entity() = default;

    // Entity should not be copied. We should not have two copies of the same entity in one session.
    Entity(const Entity&) = delete;
    void operator=(const Entity&) = delete;

    virtual std::string GetId() const = 0;
};

template <typename T, std::enable_if_t<std::is_base_of<Entity, T>::value, bool> = true>
class Repository {
public:
    Repository() = default;

    virtual ~Repository() = default;

    // The returned entity is actually borrowed from the repository, so we return a pointer.
    virtual absl::StatusOr<T*> Find(const std::string& id) = 0;

    virtual absl::Status Remove(const std::string& id) = 0;

    virtual absl::Status Save(const T& entity) = 0;
};

}  // namespace ddd
