// Copyright (c) 2023 Tencent
//     All rights reserved.
//
// Author: spockwang@tencent.com
//

#pragma once

#include <string>

namespace ddd {
namespace domain {

class Entity {
public:
    Entity() = default;

    virtual ~Entity() = default;

    virtual std::string GetId() const = 0;
};

}  // namespace domain
}  // namespace ddd
