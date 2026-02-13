#include "dsp_chain.h"

namespace dg
{

void DSPChain::process(float* l,
                       float* r,
                       size_t nframes)
{
    for (size_t i = 0; i < MAX_INSERTS; ++i)
    {
        if (m_enabled[i] && m_effects[i])
            m_effects[i]->process(l, r, nframes);
    }
}

void DSPChain::set(size_t index, DSP* dsp)
{
    if (index < MAX_INSERTS)
        m_effects[index] = dsp;
}

void DSPChain::enable(size_t index, bool en)
{
    if (index < MAX_INSERTS)
        m_enabled[index] = en;
}

}
