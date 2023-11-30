// Copyright (c) 2023 Tencent
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#pragma once

#include <string>

namespace ddd {
namespace domain {

template <typename T>
class Entity {
public:
    virtual std::string GetId() const = 0;

    virtual bool Equals(const T& other) const = 0;

protected:
    ~Entity() = default;
};

}  // namespace domain
}  // namespace ddd
