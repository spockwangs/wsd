// Copyright (c) 2019 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "hazard_pointer.h"
#include <unordered_set>

using namespace std;

namespace wsd {

HazardManager::HazardManager(int max_hp)
    : m_max_hp(max_hp)
{
}

HazardManager::HPRecType* HazardManager::AllocateHpRec()
{
    for (auto* p = m_head.load(); p; p = p->next) {
        if (p->active.test_and_set()) {
            continue;
        }
        // locked.
        return p;
    }

    std::unique_ptr<HPRecType> new_hp(new HPRecType(m_max_hp));
    new_hp->active.test_and_set();
    HPRecType* old_head = nullptr;
    do {
        old_head = m_head.load();
        new_hp->next = old_head;
    } while (!m_head.compare_exchange_weak(old_head, new_hp.get()));
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
    for (int i = 0; i < p->node_count; ++i) {
        p->nodes[i] = nullptr;
    }
    p->active.clear();
}

void HazardManager::Scan()
{
    std::unordered_set<void*> plist;
    auto* hp = m_head.load();
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
    std::list<NodeToRetire*> tmp_list;
    tmp_list.swap(hp_rec->retire_list);
    for (auto* p : tmp_list) {
        if (plist.count(p->node) > 0) {
            hp_rec->retire_list.push_back(p);
        } else {
            delete p;
        }
    }
}

void HazardManager::HelpScan()
{
    HPRecType* hp_rec = GetHpRecForCurrentThread();
    for (auto* p = m_head.load(); p; p = p->next) {
        if (p->active.test_and_set()) {
            continue;
        }
        hp_rec->retire_list.splice(hp_rec->retire_list.end(), p->retire_list);
        if (hp_rec->retire_list.size() >= 2*m_len.load()) {
            Scan();
        }
        p->active.clear();
    }
}

HazardPointer::HazardPointer(HazardManager& mgr)
    : m_mgr(mgr)
{
}

HazardPointer::~HazardPointer()
{
    Release();
}

void HazardPointer::Acquire(void* p)
{
    HazardManager::HPRecType* hp_rec = m_mgr.GetHpRecForCurrentThread();
    hp_rec->nodes[hp_rec->node_count++] = p;
    // Make the validation happens after this.
    std::atomic_thread_fence(std::memory_order_seq_cst);
}

void HazardPointer::Release()
{
    HazardManager::HPRecType* hp_rec = m_mgr.GetHpRecForCurrentThread();
    for (int i = 0; i < hp_rec->node_count; ++i) {
        hp_rec->nodes[i] = nullptr;
    }
    hp_rec->node_count = 0;
}

}  // namespace wsd
