// Copyright (c) 2019 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "ebr.h"

namespace wsd {

Ebr::Ebr()
{
}

Ebr::~Ebr()
{
    for (auto* p = m_head.load(); p;) {
        auto* q = p;
        p = p->next;
        q->DecRef();
    }
    for (int i = 0; i < 3; ++i) {
        FreeList(m_retire_list[i].load(std::memory_order_relaxed));
    }
}

void Ebr::EnterCriticalRegion()
{
    EbrRecord* p = GetEbrRecForCurrentThread();
    p->active.store(true);
    p->epoch.store(m_global_epoch.load());
}

void Ebr::ExitCriticalRegion()
{
    EbrRecord* p = GetEbrRecForCurrentThread();
    p->active.store(false);
}

Ebr::EbrRecord* Ebr::AllocateEbrRec()
{
    for (auto* p = m_head.load(std::memory_order_acquire); p; p = p->next) {
        if (p->in_use.test_and_set(std::memory_order_acquire)) {
            continue;
        }
        return p;
    }

    std::unique_ptr<EbrRecord> new_rec(new EbrRecord());
    new_rec->in_use.test_and_set(std::memory_order_relaxed);
    new_rec->IncRef();
    EbrRecord* old_head = nullptr;
    do {
        old_head = m_head.load(std::memory_order_relaxed);
        new_rec->next = old_head;
    } while (!m_head.compare_exchange_weak(old_head, new_rec.get(),
                                           std::memory_order_release,
                                           std::memory_order_relaxed));
    return new_rec.release();
}
                 
Ebr::EbrRecord* Ebr::GetEbrRecForCurrentThread()
{
    if (m_my_ebr_rec.get() == nullptr) {
        m_my_ebr_rec.reset(AllocateEbrRec());
    }
    return m_my_ebr_rec.get();
}

void Ebr::Scan()
{
    int global_epoch = m_global_epoch.load();
    for (auto* p = m_head.load(); p; p = p->next) {
        if (p->active.load() && p->epoch != global_epoch) {
            return;
        }
    }

    int next_epoch = (global_epoch + 1) % 3;
    FreeRetireList(next_epoch);
    m_global_epoch.store(next_epoch);
}

void Ebr::FreeRetireList(int epoch)
{
    NodeToRetire* head = nullptr;
    do {
        head = m_retire_list[epoch].load();
    } while (m_retire_list[epoch].compare_exchange_weak(head, nullptr));
    FreeList(head);
}

void Ebr::FreeList(NodeToRetire* head)
{
    for (auto* p = head; p;) {
        auto* q = p;
        p = p->next;
        delete q;
    }
}    

// static
void Ebr::RetireEbrRecord(EbrRecord* p)
{
    p->in_use.clear(std::memory_order_release);
    p->DecRef();
}

}  // namespace wsd
