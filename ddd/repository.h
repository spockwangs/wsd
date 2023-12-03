#pragma once

#include <memory>
#include <string>
#include <type_traits>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "entity.h"

namespace ddd {
namespace domain {

// Persistence-oriented repository interface. You should call `Save()` once the entity is changed.
template <typename T, std::enable_if_t<std::is_base_of<Entity<T>, T>::value, bool> = true>
class PersistenceRepository {
public:
    using EntityPtr = std::weak_ptr<T>;

    PersistenceRepository() = default;

    virtual ~PersistenceRepository() = default;

    PersistenceRepository(const PersistenceRepository&) = delete;
    void operator=(const PersistenceRepository&) = delete;

    PersistenceRepository(PersistenceRepository&&) = default;
    PersistenceRepository& operator=(PersistenceRepository&&) = default;

    // The returned entity is actually borrowed from the repository, so we return a pointer.
    virtual absl::StatusOr<EntityPtr> Find(const std::string& id) = 0;

    virtual absl::Status Remove(const std::string& id) = 0;

    virtual absl::Status Save(const T& entity) = 0;
};

// Collection-oriented repository interface.
//
// - `Find()` is used to reconstitute the entity.
// - To add an new entity, call `Add()`.
// - To remove an entity, call `Remove()`.
// - After the work is done, call `Commit()` to make the changes durable.
template <typename T, std::enable_if_t<std::is_base_of<Entity<T>, T>::value, bool> = true>
class CollectionRepository {
public:
    using EntityPtr = std::weak_ptr<T>;

    CollectionRepository() = default;

    virtual ~CollectionRepository() = default;

    CollectionRepository(const CollectionRepository&) = delete;
    void operator=(const CollectionRepository&) = delete;

    CollectionRepository(CollectionRepository&&) = default;
    CollectionRepository& operator=(CollectionRepository&&) = default;

    // The returned entity is actually borrowed from the repository, so we return a weak pointer.
    virtual absl::StatusOr<EntityPtr> Find(const std::string& id) = 0;

    virtual EntityPtr Add(const T& entity) = 0;

    virtual void Remove(const std::string& id) = 0;

    virtual absl::Status Commit() = 0;
};

class Order;
class LineItem;

class LazyOrderRepository {
public:
    using LazyOrderPtr = std::weak_ptr<LazyOrder>;
    using LineItemPtr = std::weak_ptr<LineItem>;
    
    virtual absl::StatusOr<OrderPtr> Find(const std::string& id) = 0;

    virtual absl::Status FindLineItems(const std::string& id, std::vector<LineItemPtr>* line_items) = 0;
    
    virtual OrderPtr AddOrder(const Order& order) = 0;

    virtual void RemoveOrder(const std::string& id) = 0;

    virtual LineItemPtr AddLineItem(const LineItem& line_item) = 0;

    virtual void RemoveLineItem(const std::string& order_id, const std::string& id) = 0;
    
    virtual absl::Status Commit() = 0;
    
protected:
    ~LazyOrderRepository() = default;
};

}  // namespace domain
}  // namespace ddd
