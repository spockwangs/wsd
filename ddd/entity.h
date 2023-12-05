// Copyright (c) 2023 Tencent
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#pragma once

#include <functional>
#include <string>
#include <type_traits>
#include <utility>

namespace ddd {
namespace domain {

// An entity should have a primary key, which implements `std::hash<>` and `operator==()` and
// eligible to be in an associated container.
template <typename ID, typename T, typename = decltype(std::hash<ID>(), std::declval<ID>() == std::declval<ID>())>
class Entity {
public:
    virtual ID GetId() const = 0;

    virtual bool Equals(const T& other) const = 0;

protected:
    ~Entity() = default;
};

}  // namespace domain
}  // namespace ddd
