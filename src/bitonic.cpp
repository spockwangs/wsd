// Copyright (c) 2021 spockwang.
//     All rights reserved.
//
// Author: wbbtiger@gmail.com
//

#include "wsd/bitonic.h"
#include <atomic>
#include <stdexcept>
#include <vector>

namespace wsd {

class Balancer final {
public:
    Balancer()
        : m_toggle(true)
    {}

    Balancer(const Balancer&) = delete;
    Balancer& operator=(const Balancer&) = delete;
    
    int Traverse(int)
    {
        for (;;) {
            bool toggle = m_toggle.load();
            if (m_toggle.compare_exchange_weak(toggle, !toggle)) {
                if (toggle) {
                    return 0;
                } else {
                    return 1;
                }
            }
        }
    }
    
private:
    std::atomic<bool> m_toggle;
};

class Merger final {
public:
    explicit Merger(int width)
        : m_width(width)
    {
        if ((width % 2) != 0) {
            throw std::invalid_argument("width should be even");
        }
        
        m_layer.reset(new Balancer[width/2]);
        if (width > 2) {
            m_half0.reset(new Merger(width/2));
            m_half1.reset(new Merger(width/2));
        }
    }

    Merger(const Merger&) = delete;
    Merger& operator=(const Merger&) = delete;

    int Traverse(int input)
    {
        if (input > m_width) {
            throw std::invalid_argument("bad input");
        }

        int output = 0;
        if (m_width > 2) {
            if (input < m_width/2) {
                if ((input % 2) == 0) {
                    output = m_half0->Traverse(input/2);
                } else {
                    output = m_half1->Traverse(input/2);
                }
            } else {
                if ((input%2) == 0) {
                    output = m_half1->Traverse(input/2);
                } else {
                    output = m_half0->Traverse(input / 2);
                }
            }
        }
        output = m_layer[output].Traverse(0) + output * 2;
        return output;
    }

private:
    std::unique_ptr<Merger> m_half0;
    std::unique_ptr<Merger> m_half1;
    std::unique_ptr<Balancer[]> m_layer;
    const int m_width;
};

Bitonic::Bitonic(int width)
    : m_width(width)
{
    if ((width % 2) != 0) {
        throw std::invalid_argument("width should be even");
    }
    
    m_merger.reset(new Merger(width));
    if (width > 2) {
        m_half0.reset(new Bitonic(width/2));
        m_half1.reset(new Bitonic(width/2));
    }
}

Bitonic::~Bitonic()
{
}

int Bitonic::Traverse(int input)
{
    if (input > m_width) {
        throw std::invalid_argument("bad input");
    }
    
    int output = 0;
    if (m_width > 2) {
        if (input < m_width/2) {
            output = m_half0->Traverse(input);
        } else {
            output = m_half1->Traverse(input - m_width / 2);
        }
    }
    return m_merger->Traverse(output + input / (m_width/2) * (m_width/2));
}
        

}  // namespace wsd
