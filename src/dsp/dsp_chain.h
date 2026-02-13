#pragma once

#include "dsp.h"
#include <array>

namespace dg
{

constexpr size_t MAX_INSERTS = 4;

class DSPChain
{
public:
    void process(float* l,
                 float* r,
                 size_t nframes);

    void set(size_t index, DSP* dsp);
    void enable(size_t index, bool en);

private:
    std::array<DSP*, MAX_INSERTS> m_effects{};
    std::array<bool, MAX_INSERTS> m_enabled{};
};

}
