// Copyright (c) 2019 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "hazard_pointer.h"
#include <atomic>
#include <utility>
#include <future>
#include "gtest/gtest.h"

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

    // The destructors of hp1 and hp2 should release automatically.
    EXPECT_EQ(0, hp_mgr.TEST_GetNumberOfHp());            
}

TEST(HazardPointer, SequentialExecution3)
{
    wsd::HazardManager hp_mgr(1);
    wsd::HazardPointer hp1(hp_mgr, 0);
    int* p = new int;
    hp1.Acquire(p);
    EXPECT_TRUE(hp_mgr.TEST_HpListContains(p));
    EXPECT_FALSE(hp_mgr.TEST_RetireListContains(p));

    hp1.Release();
    EXPECT_FALSE(hp_mgr.TEST_HpListContains(p));
    EXPECT_FALSE(hp_mgr.TEST_RetireListContains(p));

    hp_mgr.RetireNode(p);
    EXPECT_TRUE(hp_mgr.TEST_RetireListContains(p));
    
    p = new int;
    hp_mgr.RetireNode(p);
    // Now the retire list of current thread should have been reclaimed.
    EXPECT_FALSE(hp_mgr.TEST_RetireListContains(p));
    EXPECT_EQ(0, hp_mgr.TEST_GetRetireListLenOfCurrentThread());
}

TEST(HazardPointer, ConcurrentExecution)
{
    wsd::HazardManager hp_mgr(1);
    int *p = new int;

    std::promise<void> go;
    std::shared_future<void> ready(go.get_future());
    std::promise<void> thread1_ready, thread1_ready2;
    std::promise<void> thread2_ready, thread2_ready2;
    std::promise<void> go2;
    std::shared_future<void> ready2(go2.get_future());

    std::future<void> done1 = std::async(std::launch::async,
                                         [&, ready, p, ready2] () {
                                             wsd::HazardPointer hp(hp_mgr, 0);
                                             thread1_ready.set_value();
                                             ready.wait();
                                             hp.Acquire(p);

                                             thread1_ready2.set_value();
                                             ready2.wait();
                                         });
    std::future<void> done2 = std::async(std::launch::async,
                                         [&, ready, p, ready2] ()
                                         {
                                             wsd::HazardPointer hp(hp_mgr, 0);
                                             thread2_ready.set_value();
                                             ready.wait();
                                             hp.Acquire(p);

                                             thread2_ready2.set_value();
                                             ready2.wait();
                                         });

    thread1_ready.get_future().wait();
    thread2_ready.get_future().wait();
    go.set_value();

    thread1_ready2.get_future().wait();
    thread2_ready2.get_future().wait();
    EXPECT_TRUE(hp_mgr.TEST_HpListContains(p));
    EXPECT_EQ(2, hp_mgr.TEST_GetNumberOfHp());

    go2.set_value();
    done1.wait();
    done2.wait();
    EXPECT_FALSE(hp_mgr.TEST_HpListContains(p));    
    EXPECT_EQ(0, hp_mgr.TEST_GetNumberOfHp());
    delete p;
}

TEST(HazardPointer, ConcurrentExecution2)
{
    wsd::HazardManager hp_mgr(1);
    int *p = new int;

    std::promise<void> go;
    std::shared_future<void> ready(go.get_future());
    std::promise<void> thread1_ready, thread1_ready2;
    std::promise<void> thread2_ready, thread2_ready2;
    std::promise<void> go2;
    std::shared_future<void> ready2(go2.get_future());

    std::future<void> done1 = std::async(std::launch::async,
                                         [&, ready, p, ready2] () {
                                             wsd::HazardPointer hp(hp_mgr, 0);
                                             thread1_ready.set_value();
                                             ready.wait();
                                             hp.Acquire(p);

                                             thread1_ready2.set_value();
                                             ready2.wait();
                                         });
    std::future<void> done2 = std::async(std::launch::async,
                                         [&, ready, p, ready2] ()
                                         {
                                             wsd::HazardPointer hp(hp_mgr, 0);
                                             thread2_ready.set_value();
                                             ready.wait();
                                             hp.Acquire(p);
                                             hp.Release();
                                             
                                             thread2_ready2.set_value();
                                             ready2.wait();
                                             hp_mgr.RetireNode(p);
                                         });

    thread1_ready.get_future().wait();
    thread2_ready.get_future().wait();
    go.set_value();

    thread1_ready2.get_future().wait();
    thread2_ready2.get_future().wait();
    EXPECT_TRUE(hp_mgr.TEST_HpListContains(p));
    EXPECT_EQ(1, hp_mgr.TEST_GetNumberOfHp());

    go2.set_value();
    done1.wait();
    done2.wait();
    EXPECT_FALSE(hp_mgr.TEST_HpListContains(p));    
    EXPECT_EQ(0, hp_mgr.TEST_GetNumberOfHp());
}
