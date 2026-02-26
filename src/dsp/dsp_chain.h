#pragma once

#include "dsp.h"
#include <array>

namespace disgrace_ns
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

    const ::std::array<disgrace_ns::DSP*, MAX_INSERTS>& effects() const { return m_effects; } // Add this line

private:
    ::std::array<disgrace_ns::DSP*, MAX_INSERTS> m_effects{};
    ::std::array<bool, MAX_INSERTS> m_enabled{};
};

} // namespace disgrace_ns
