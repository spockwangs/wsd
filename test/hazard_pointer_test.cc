// Copyright (c) 2019 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "hazard_pointer.h"
#include <atomic>
#include <utility>
#include "gtest/gtest.h"

template <typename T>
class FifoQueue {
public:
    FifoQueue()
    {
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
    wsd::HazardManager m_hazard_manager{2};
};

template <typename T>
void FifoQueue<T>::Enqueue(T&& data)
{
    wsd::HazardPointer hp(m_hazard_manager);
    NodeType* node = new NodeType();
    node->data = std::forward<T>(data);
    node->next = nullptr;
    NodeType* t = nullptr;
    while (true) {
        t = m_tail.load();
        hp.Acquire(t);
        if (m_tail.load() != t)
            continue;
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
    wsd::HazardPointer hp(m_hazard_manager, 0);
    wsd::HazardPointer hp1(m_hazard_manager, 1);
    NodeType* h = nullptr;
    while (true) {
        h = m_head.load();
        hp.Acquire(h);
        if (m_head.load() != h)
            continue;
        NodeType* t = m_tail.load();
        NodeType* next = h->next.load();
        hp1.Acquire(next);
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
    m_hazard_manager.RetireNode(h);
    return true;
}

TEST(HazardPointer, fifo_queue)
{
    FifoQueue<int> queue;
    queue.Enqueue(1);
    int a;
    EXPECT_TRUE(queue.Dequeue(&a));
    EXPECT_EQ(1, a);
}

TEST(HazardPointer, SequentialExecution)
{
    wsd::HazardManager hp_mgr(1);
    wsd::HazardPointer hp(hp_mgr, 0);
    int a;
    hp.Acquire(&a);
    EXPECT_EQ(1, hp_mgr.TEST_GetNumberOfHp());
    hp.Release();
    EXPECT_EQ(0, hp_mgr.TEST_GetNumberOfHp());

    EXPECT_THROW(wsd::HazardPointer(hp_mgr, 1), std::invalid_argument);
}

TEST(HazardPointer, SequentialExecution2)
{
    wsd::HazardManager hp_mgr(2);
    {
        wsd::HazardPointer hp1(hp_mgr, 0);
        wsd::HazardPointer hp2(hp_mgr, 1);
        EXPECT_THROW(wsd::HazardPointer(hp_mgr, 2), std::invalid_argument);

        int a, b;
        hp1.Acquire(&a);
        EXPECT_EQ(1, hp_mgr.TEST_GetNumberOfHp());
        hp1.Release();
        EXPECT_EQ(0, hp_mgr.TEST_GetNumberOfHp());
        hp1.Acquire(&a);
        EXPECT_EQ(1, hp_mgr.TEST_GetNumberOfHp());
        hp2.Acquire(&b);
        EXPECT_EQ(2, hp_mgr.TEST_GetNumberOfHp());
        hp2.Release();
        EXPECT_EQ(1, hp_mgr.TEST_GetNumberOfHp());
        hp1.Release();
        EXPECT_EQ(0, hp_mgr.TEST_GetNumberOfHp());

        hp2.Acquire(&a);
        hp1.Acquire(&b);
        EXPECT_EQ(2, hp_mgr.TEST_GetNumberOfHp());        
    }
    EXPECT_EQ(0, hp_mgr.TEST_GetNumberOfHp());            
}
