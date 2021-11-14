// Copyright (c) 2021 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>

namespace wsd {

class CombiningTree final {
public:
    CombiningTree(int width);

    CombiningTree(const CombiningTree&) = delete;
    CombiningTree& operator=(const CombiningTree&) = delete;

    ~CombiningTree();
    
    int GetAndIncrement(int tid);

    int Get();

private:
    class Node;
    std::vector<Node*> m_nodes;
    std::vector<Node*> m_leaf;
};
    
}  // namespace wsd
