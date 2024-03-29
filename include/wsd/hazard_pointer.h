// Copyright (c) 2019 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#pragma once

#include <atomic>
#include <boost/checked_delete.hpp>
#include <boost/thread/tss.hpp>
#include <cassert>
#include <functional>
#include <list>
#include <memory>
#include <vector>

namespace wsd {

class HazardManager final {
private:
    template <typename T>
    static void DoDelete(void* p)
    {
        boost::checked_delete(static_cast<T*>(p));
    }

    struct NodeToRetire {
        void* node;
        std::function<void(void*)> deleter;

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

public:
    HazardManager(int max_hp);

    ~HazardManager();

    template <typename T>
    void RetireNode(T* node)
    {
        HPRecType* hp_rec = GetHpRecForCurrentThread();
        hp_rec->retire_list.emplace_back(node);
        if (hp_rec->retire_list.size() >= 2 * m_len.load()) {
            Scan();
            HelpScan();
        }
    }

    int TEST_GetNumberOfHp() const;

    bool TEST_HpListContains(void* p) const;

    bool TEST_RetireListContains(void* p) const;

    int TEST_GetRetireListLenOfCurrentThread() const;

private:
    struct HPRecType {
        std::atomic_flag active = ATOMIC_FLAG_INIT;
        std::atomic<int> ref_count{0};
        std::unique_ptr<void*[]> nodes;
        const int num_of_nodes = 0;
        HPRecType* next = nullptr;
        std::list<NodeToRetire> retire_list;

        HPRecType(int max_hp) : ref_count(1), nodes(new void*[max_hp]), num_of_nodes(max_hp)
        {
            for (int i = 0; i < max_hp; ++i) {
                nodes[i] = nullptr;
            }
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

    HPRecType* AllocateHpRec();

    HPRecType* GetHpRecForCurrentThread();

    static void RetireHpRec(HPRecType* p);

    void Scan();

    void HelpScan();

    const int m_max_hp;
    std::atomic<HPRecType*> m_head{nullptr};
    std::atomic<int> m_len{0};
    boost::thread_specific_ptr<HPRecType> m_my_hp_rec{&HazardManager::RetireHpRec};

    friend class HazardPointer;
};

class HazardPointer final {
public:
    HazardPointer(HazardManager& mgr, int i = 0);

    ~HazardPointer();

    void Acquire(void* p);

    void Release();

private:
    void** m_hp = nullptr;
};

}  // namespace wsd
