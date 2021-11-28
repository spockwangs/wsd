// Copyright (c) 2021 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include <memory>

namespace wsd {

class Merger;

class Bitonic final {
public:
    explicit Bitonic(int width);

    ~Bitonic();
    
    Bitonic(const Bitonic&) = delete;
    Bitonic& operator=(const Bitonic&) = delete;

    int Traverse(int input);
    
private:
    std::unique_ptr<Bitonic> m_half0;
    std::unique_ptr<Bitonic> m_half1;
    std::unique_ptr<Merger> m_merger;
    const int m_width;
};

}  // namespace wsd
