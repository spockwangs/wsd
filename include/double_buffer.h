// Copyright (c) 2019 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//
// @brief 双缓冲作为一种MVCC机制（最多2个版本）适合于数据变化频率较低的并发数据结构。
//
// 仅支持单写多读。
// 

#pragma once

#include <atomic>
#include <thread>

namespace wsd {

template <typename T> class DoubleBufferReader;
template <typename T> class DoubleBufferWriter;

// T must satisfy DefaultConstructible.
template <typename T>
class DoubleBuffer final {
public:
    typedef DoubleBufferReader<T> Reader;
    typedef DoubleBufferWriter<T> Writer;
    
    DoubleBuffer()
    {
    }

private:
    DoubleBuffer(const DoubleBuffer&) = delete;
    void operator=(const DoubleBuffer&) = delete;
    
    struct RefCountedData {
        T data;
        std::atomic<int> reader_cnt{0};
    };
    
    RefCountedData* AcquireForReading()
    {
        int idx = 0;
        while (true) {
            idx = m_read_idx.load(std::memory_order_relaxed);
            m_data[idx].reader_cnt.fetch_add(1, std::memory_order_relaxed);
            // Although |m_read_idx| may change multiple times and change back to |idx| between the
            // above loading and this loading, it does not matter. As long as it changes back to
            // |idx|, it means the data pointed to by |idx| has complete updating and can be read.
            if (idx == m_read_idx.load(std::memory_order_acquire))
                break;
            ReleaseReading(idx);
        }
        return &m_data[idx];
    }
        
    static void ReleaseReading(RefCountedData* p)
    {
        p->reader_cnt.fetch_sub(1, std::memory_order_release);
    }

    RefCountedData* AcquireForWriting()
    {
        for (;;) {
            int idx = 1 - m_read_idx.load(std::memory_order_relaxed);
            if (m_data[idx].reader_cnt.load(std::memory_order_acquire) == 0) {
                return m_data[idx];
            }
            std::this_thread::yield();
        }
    }

    void ReleaseWriting()
    {
        int idx = 1 - m_read_idx.load(std::memory_order_relaxed);
        m_read_idx.store(idx, std::memory_order_release);
    }
    
    std::atomic<int> m_read_idx{0};
    RefCountedData m_data[2];

    friend class DoubleBufferReader<T>;
    friend class DoubleBufferWriter<T>;
};

template <typename T>
class DoubleBufferReader final {
public:
    DoubleBufferReader(DoubleBuffer<T>* db_p)
        : m_data(db_p->AcquireForReading())
    {
    }
    
    ~DoubleBufferReader()
    {
        DoubleBuffer<T>::Release(m_data);
    }
        
    const T& Get() const
    {
        return m_data->data;
    }

private:
    DoubleBufferReader(const DoubleBufferReader&) = delete;
    void operator=(const DoubleBufferReader&) = delete;

    typename DoubleBuffer<T>::RefCountedData* m_data;

    friend class DoubleBuffer<T>;
};

template <typename T>
class DoubleBufferWriter final {
public:
    DoubleBufferWriter(DoubleBuffer<T>* db_p)
        : m_db_p(db_p),
          m_data_p(m_db_p->AcquireForWriting())
    {
    }

    ~DoubleBufferWriter()
    {
        if (m_db_p)
            m_db_p->ReleaseWriting();
    }
    
    T* Get()
    {
        assert(m_data_p);
        return &m_data_p->data;
    }

private:
    DoubleBufferWriter(const DoubleBufferWriter&) = delete;
    void operator=(const DoubleBufferWriter&) = delete;
    
    DoubleBuffer<T>* m_db_p = nullptr;
    typename DoubleBuffer<T>::RefCountedData* m_data_p = nullptr;

    friend class DoubleBuffer<T>;
};

}  // namespace wsd
