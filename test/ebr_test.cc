// Copyright (c) 2019 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "ebr.h"
#include <atomic>
#include <utility>
#include <future>
#include "gtest/gtest.h"

namespace {

template <typename T>
class FifoQueue {
public:
    FifoQueue()
    {
    }

    ~FifoQueue()
    {
        for (auto* p = m_head.load(); p;) {
            auto* q = p;
            p = p->next;
            delete q;
        }
    }
    
    void Enqueue(T&& t);

    bool Dequeue(T* p);

private:
    struct NodeType {
        T data;
        std::atomic<NodeType*> next{nullptr};
    };

    std::atomic<NodeType*> m_head{new NodeType()};
    std::atomic<NodeType*> m_tail{m_head.load()};
    wsd::EbrManager m_ebr;
};

template <typename T>
void FifoQueue<T>::Enqueue(T&& data)
{
    wsd::EbrGuard ebr_guard(m_ebr);
    NodeType* node = new NodeType();
    node->data = std::forward<T>(data);
    node->next = nullptr;
    NodeType* t = nullptr;
    while (true) {
        t = m_tail.load();
        NodeType* next = t->next.load();
        if (m_tail.load() != t) {
            continue;
        }
        if (next != nullptr) {
            m_tail.compare_exchange_weak(t, next);
            continue;
        }
        NodeType* null_ptr = nullptr;
        if (t->next.compare_exchange_weak(null_ptr, node)) {
            break;
        }
    }
    m_tail.compare_exchange_strong(t, node);
}

template <typename T>
bool FifoQueue<T>::Dequeue(T* p)
{
    NodeType* h = nullptr;
    while (true) {
        h = m_head.load();
        NodeType* t = m_tail.load();
        NodeType* next = h->next.load();
        if (h != m_head.load())
            continue;
        if (next == nullptr)
            return false;
        if (h == t) {
            m_tail.compare_exchange_weak(t, next);
            continue;
        }
        *p = next->data;
        if (m_head.compare_exchange_weak(h, next))
            break;
    }
    m_ebr.RetireNode(h);
    return true;
}

}  // namespace

TEST(Ebr, fifo_queue)
{
    FifoQueue<int> queue;
    queue.Enqueue(1);
    int a;
    EXPECT_TRUE(queue.Dequeue(&a));
    EXPECT_EQ(1, a);
}
