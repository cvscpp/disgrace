#pragma once
#include <cstddef>

namespace dg
{

class DSP
{
public:
    virtual ~DSP() = default;

    virtual void process(float* l,
                         float* r,
                         size_t nframes) = 0;
};

    virtual size_t latency() const { return 0; }


}
