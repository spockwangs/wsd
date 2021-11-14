// Copyright (c) 2019 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "hazard_pointer.h"

#include <exception>
#include <unordered_set>

using namespace std;

namespace wsd {

HazardManager::HazardManager(int max_hp) : m_max_hp(max_hp)
{
}

HazardManager::~HazardManager()
{
    // The hazard pointer list should contain no hazard pointers now. Decrement references to all
    // hazard pointers.
    for (auto* p = m_head.load(std::memory_order_acquire); p;) {
        auto* q = p;
        p = p->next;
        q->DecRef();
    }
}

int HazardManager::TEST_GetNumberOfHp() const
{
    int hp_count = 0;
    for (auto* p = m_head.load(std::memory_order_acquire); p; p = p->next) {
        for (int i = 0; i < m_max_hp; ++i) {
            if (p->nodes[i]) {
                ++hp_count;
            }
        }
    }
    return hp_count;
}

bool HazardManager::TEST_HpListContains(void* p) const
{
    for (auto* q = m_head.load(std::memory_order_acquire); q; q = q->next) {
        for (int i = 0; i < q->num_of_nodes; ++i) {
            if (q->nodes[i] == p) {
                return true;
            }
        }
    }
    return false;
}

bool HazardManager::TEST_RetireListContains(void* p) const
{
    for (auto* q = m_head.load(std::memory_order_acquire); q; q = q->next) {
        for (const auto& node : q->retire_list) {
            if (node.node == p) {
                return true;
            }
        }
    }
    return false;
}

int HazardManager::TEST_GetRetireListLenOfCurrentThread() const
{
    auto* hp_rec = m_my_hp_rec.get();
    if (hp_rec) {
        return hp_rec->retire_list.size();
    }
    return 0;
}

HazardManager::HPRecType* HazardManager::AllocateHpRec()
{
    for (auto* p = m_head.load(std::memory_order_acquire); p; p = p->next) {
        if (p->active.test_and_set(std::memory_order_acquire)) {
            continue;
        }
        // locked.
        return p;
    }

    std::unique_ptr<HPRecType> new_hp(new HPRecType(m_max_hp));
    new_hp->active.test_and_set(std::memory_order_relaxed);
    new_hp->IncRef();
    new_hp->next = m_head.load(std::memory_order_relaxed);
    while (!m_head.compare_exchange_weak(new_hp->next, new_hp.get(), std::memory_order_release,
                                         std::memory_order_relaxed))
        ;
    m_len.fetch_add(m_max_hp, std::memory_order_relaxed);
    return new_hp.release();
}

HazardManager::HPRecType* HazardManager::GetHpRecForCurrentThread()
{
    if (m_my_hp_rec.get() == nullptr) {
        m_my_hp_rec.reset(AllocateHpRec());
    }
    return m_my_hp_rec.get();
}

// static
void HazardManager::RetireHpRec(HazardManager::HPRecType* p)
{
    for (int i = 0; i < p->num_of_nodes; ++i) {
        p->nodes[i] = nullptr;
    }
    p->active.clear(std::memory_order_release);
    p->DecRef();
}

void HazardManager::Scan()
{
    std::unordered_set<void*> plist;
    auto* hp = m_head.load(std::memory_order_acquire);
    while (hp != nullptr) {
        for (int i = 0; i < m_max_hp; ++i) {
            auto* p = hp->nodes[i];
            if (p) {
                plist.insert(p);
            }
        }
        hp = hp->next;
    }

    HPRecType* hp_rec = GetHpRecForCurrentThread();
    std::list<NodeToRetire> tmp_list;
    tmp_list.swap(hp_rec->retire_list);
    for (auto& n : tmp_list) {
        if (plist.count(n.node) > 0) {
            hp_rec->retire_list.push_back(std::move(n));
        }
    }
}

void HazardManager::HelpScan()
{
    HPRecType* hp_rec = GetHpRecForCurrentThread();
    for (auto* p = m_head.load(std::memory_order_acquire); p; p = p->next) {
        if (p->active.test_and_set(std::memory_order_acquire)) {
            continue;
        }
        hp_rec->retire_list.splice(hp_rec->retire_list.end(), p->retire_list);
        if (hp_rec->retire_list.size() >= 2 * m_len.load(std::memory_order_relaxed)) {
            Scan();
        }
        p->active.clear(std::memory_order_release);
    }
}

HazardPointer::HazardPointer(HazardManager& mgr, int i)
{
    HazardManager::HPRecType* hp_rec = mgr.GetHpRecForCurrentThread();
    if (i >= mgr.m_max_hp) {
        throw std::invalid_argument("input number bigger than number of hazard pointers");
    }
    m_hp = &hp_rec->nodes[i];
}

HazardPointer::~HazardPointer()
{
    Release();
}

void HazardPointer::Acquire(void* p)
{
    *m_hp = p;
    // Make the validation happens after this.
    std::atomic_thread_fence(std::memory_order_seq_cst);
}

void HazardPointer::Release()
{
    *m_hp = nullptr;
}

}  // namespace wsd
