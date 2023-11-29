#pragma once

#include <memory>
#include <string>
#include <type_traits>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "entity.h"

namespace ddd {

template <typename T, std::enable_if_t<std::is_base_of<Entity, T>::value, bool> = true>
class Repository {
public:
    using EntityPtr = std::weak_ptr<T>;

    Repository() = default;

    virtual ~Repository() = default;

    // The returned entity is actually borrowed from the repository, so we return a pointer.
    virtual absl::StatusOr<EntityPtr> Find(const std::string& id) = 0;

    virtual absl::Status Remove(const std::string& id) = 0;

    virtual absl::Status Save(const T& entity) = 0;
};

}  // namespace ddd
