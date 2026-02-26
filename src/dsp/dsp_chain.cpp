#include "dsp_chain.h"

namespace disgrace_ns
{

void disgrace_ns::DSPChain::process(float* l,
                       float* r,
                       size_t nframes)
{
    for (size_t i = 0; i < MAX_INSERTS; ++i)
    {
        if (m_enabled[i] && m_effects[i])
            m_effects[i]->process(l, r, nframes);
    }
}

void disgrace_ns::DSPChain::set(size_t index, disgrace_ns::DSP* dsp)
{
    if (index < MAX_INSERTS)
        m_effects[index] = dsp;
}

void disgrace_ns::DSPChain::enable(size_t index, bool en)
{
    if (index < MAX_INSERTS)
        m_enabled[index] = en;
}

} // namespace disgrace_ns
