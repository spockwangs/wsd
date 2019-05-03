// Copyright (c) 2019 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
// 
// The Epoch-based reclamation.

#pragma once

#include <atomic>
#include <functional>
#include "boost/thread/tss.hpp"
#include "boost/checked_delete.hpp"

namespace wsd {

class Ebr final {
public:
    Ebr();

    // Disallow copy and assignment.
    Ebr(const Ebr&) = delete;
    void operator=(const Ebr&) = delete;
    
    ~Ebr();
    
    void EnterCriticalRegion();

    void ExitCriticalRegion();

    template <typename T>
    void RetireNode(T* node)
    {
        EbrRecord* p = GetEbrRecForCurrentThread();
        std::unique_ptr<NodeToRetire> node_to_retire(new NodeToRetire(node));
        NodeToRetire* head = nullptr;
        do {
            head = m_retire_list[p->epoch].load();
            node_to_retire->next = head;
        } while (m_retire_list[p->epoch].compare_exchange_weak(head, node_to_retire.get()));
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

        EbrRecord()
            : ref_count(1),
              active(true),
              epoch(0)
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
        NodeToRetire(T* p)
            : node(p),
              deleter(&DoDelete<T>)
        {
        }

        ~NodeToRetire()
        {
            deleter(node);
        }

        NodeToRetire(const NodeToRetire&) = delete;

        NodeToRetire(NodeToRetire&& other)
            : node(other.node),
              deleter(std::move(other.deleter))
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
    boost::thread_specific_ptr<EbrRecord> m_my_ebr_rec{&Ebr::RetireEbrRecord};
};

}  // namespace wsd
