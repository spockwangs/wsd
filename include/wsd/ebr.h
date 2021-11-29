// Copyright (c) 2019 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//
// The Epoch-based reclamation.

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <assert.h>

#include "boost/checked_delete.hpp"
#include "boost/thread/tss.hpp"

namespace wsd {

class EbrManager final {
public:
    EbrManager();

    // Disallow copy and assignment.
    EbrManager(const EbrManager&) = delete;
    void operator=(const EbrManager&) = delete;

    ~EbrManager();

    void EnterCriticalRegion();

    void ExitCriticalRegion();

    template <typename T>
    void RetireNode(T* node)
    {
        EbrRecord* p = GetEbrRecForCurrentThread();
        std::unique_ptr<NodeToRetire> node_to_retire(new NodeToRetire(node));
        node_to_retire->next = m_retire_list[p->epoch].load(std::memory_order_relaxed);
        while (!m_retire_list[p->epoch].compare_exchange_weak(node_to_retire->next, node_to_retire.get(),
                                                              std::memory_order_release, std::memory_order_relaxed))
            ;
        node_to_retire.release();
        Scan();
    }

private:
    struct EbrRecord {
        std::atomic_flag in_use = ATOMIC_FLAG_INIT;
        EbrRecord* next = nullptr;
        std::atomic<int> ref_count{0};
        std::atomic<bool> active{false};
        std::atomic<int> epoch{0};

        EbrRecord() : ref_count(1), active(true), epoch(0)
        {
        }

        void IncRef()
        {
            ref_count.fetch_add(1, std::memory_order_relaxed);
        }

        void DecRef()
        {
            int old = ref_count.fetch_sub(1, std::memory_order_relaxed);
            assert(old >= 1);
            if (old <= 1) {
                delete this;
            }
        }
    };

    template <typename T>
    static void DoDelete(void* p)
    {
        boost::checked_delete(static_cast<T*>(p));
    }

    struct NodeToRetire {
        void* node = nullptr;
        std::function<void(void*)> deleter;
        NodeToRetire* next = nullptr;

        template <typename T>
        NodeToRetire(T* p) : node(p), deleter(&DoDelete<T>)
        {
        }

        ~NodeToRetire()
        {
            deleter(node);
        }

        NodeToRetire(const NodeToRetire&) = delete;

        NodeToRetire(NodeToRetire&& other) : node(other.node), deleter(std::move(other.deleter))
        {
            other.node = nullptr;
        }

        void operator=(const NodeToRetire&) = delete;

        NodeToRetire& operator=(NodeToRetire&& other)
        {
            if (this != &other) {
                node = other.node;
                other.node = nullptr;
                deleter = std::move(other.deleter);
            }
            return *this;
        }
    };

    EbrRecord* AllocateEbrRec();

    EbrRecord* GetEbrRecForCurrentThread();

    void Scan();

    void FreeRetireList(int epoch);

    void FreeList(NodeToRetire* head);

    static void RetireEbrRecord(EbrRecord* p);

    std::atomic<EbrRecord*> m_head{nullptr};
    std::atomic<int> m_global_epoch{0};
    std::atomic<NodeToRetire*> m_retire_list[3]{{nullptr}};
    boost::thread_specific_ptr<EbrRecord> m_my_ebr_rec{&EbrManager::RetireEbrRecord};
};

class EbrGuard final {
public:
    EbrGuard(EbrManager& ebr) : m_ebr(ebr)
    {
        m_ebr.EnterCriticalRegion();
    }

    EbrGuard(const EbrGuard&) = delete;
    void operator=(const EbrGuard&) = delete;

    ~EbrGuard()
    {
        m_ebr.ExitCriticalRegion();
    }

private:
    EbrManager& m_ebr;
};

}  // namespace wsd
