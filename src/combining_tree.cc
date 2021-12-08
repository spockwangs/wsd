// Copyright (c) 2021 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "wsd/combining_tree.h"
#include <utility>
#include <stack>
#include <atomic>
#include "wsd/util.h"

using namespace std;

namespace wsd {

class CombiningTree::Node {
public:
    Node() = default;

    Node(Node* parent)
        : m_status(IDLE),
          m_parent(parent)
    {
    }

    Node* Parent() const
    {
        return m_parent;
    }

    bool PreCombine()
    {
        std::unique_lock<std::mutex> lock(m_mu);
        while (m_locked) {
            m_cond.wait(lock);
        }

        switch (m_status) {
        case IDLE:
            m_status = FIRST;
            return true;
        case FIRST:
            m_locked = true;
            m_status = SECOND;
            return false;
        case ROOT:
            return false;
        default:
            throw std::logic_error("unexpected Node status");
        }
        return false;
    }

    int Combine(int combined)
    {
        std::unique_lock<std::mutex> lock(m_mu);
        while (m_locked) {
            m_cond.wait(lock);
        }
        m_locked = true;
        m_first_value = combined;
        switch (m_status) {
        case FIRST:
            return m_first_value;
        case SECOND:
            return m_first_value + m_second_value;
        default:
            throw std::logic_error("unexpected Node status");
        }
    }

    int Op(int combined)
    {
        std::unique_lock<std::mutex> lock(m_mu);
        switch (m_status) {
        case ROOT: {
            int prior = m_result;
            m_result += combined;
            return prior;
        }
        case SECOND:
            m_second_value = combined;
            m_locked = false;
            m_cond.notify_all();
            while (m_status != RESULT) {
                m_cond.wait(lock);
            }
            m_locked = false;
            m_cond.notify_all();
            m_status = IDLE;
            return m_result;
        default:
            throw std::logic_error("unexpected Node status");
        }
    }

    void Distribute(int prior)
    {
        std::unique_lock<std::mutex> lock(m_mu);
        switch (m_status) {
        case FIRST:
            m_status = IDLE;
            m_locked = false;
            break;
        case SECOND:
            m_result = prior + m_first_value;
            m_status = RESULT;
            break;
        default:
            throw std::logic_error("unexpected Node status");
        }
        m_cond.notify_all();
    }

    int Result()
    {
        std::unique_lock<std::mutex> lock(m_mu);
        return m_result;
    }
    
private:
    enum Status {
        IDLE, FIRST, SECOND, RESULT, ROOT
    };

    std::mutex m_mu;
    std::condition_variable m_cond;
    bool m_locked = false;
    Status m_status = ROOT;
    int m_first_value = 0;
    int m_second_value = 0;
    int m_result = 0;
    Node* m_parent = nullptr;
};

CombiningTree::CombiningTree(int width)
{
    m_nodes.push_back(new Node());
    for (int i = 1; i < width; ++i) {
        m_nodes.push_back(new Node(m_nodes[(i-1)/2]));
    }
    for (int i = 0; i < (width+1)/2; ++i) {
        m_leaf.push_back(m_nodes[m_nodes.size()-i-1]);
    }
}

CombiningTree::~CombiningTree()
{
    for (auto* p : m_nodes) {
        delete p;
    }
}

int CombiningTree::GetAndIncrement()
{
    int tid = ThreadId();
    stack<Node*> stack;
    Node* my_leaf = m_leaf[tid/2];
    Node* node = my_leaf;
    while (node->PreCombine()) {
        node = node->Parent();
    }
    Node* stop = node;

    node = my_leaf;
    int combined = 1;
    while (node != stop) {
        combined = node->Combine(combined);
        stack.push(node);
        node = node->Parent();
    }

    int prior = stop->Op(combined);

    while (!stack.empty()) {
        node = stack.top();
        stack.pop();
        node->Distribute(prior);
    }
    return prior;
}

int CombiningTree::Get()
{
    return m_nodes[0]->Result();
}
    
}  // namespace wsd
