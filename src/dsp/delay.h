#pragma once
#include "dsp.h"
#include <array>

namespace dg
{

class DelayDSP : public DSP
{
public:
    static constexpr size_t MAX_DELAY = 48000;

    float feedback = 0.4f;
    float mix = 0.3f;

    void process(float* l,
                 float* r,
                 size_t nframes) override
    {
        for (size_t i = 0; i < nframes; ++i)
        {
            float dl = m_buffer_l[m_pos];
            float dr = m_buffer_r[m_pos];

            m_buffer_l[m_pos] = l[i] + dl * feedback;
            m_buffer_r[m_pos] = r[i] + dr * feedback;

            l[i] = l[i] * (1 - mix) + dl * mix;
            r[i] = r[i] * (1 - mix) + dr * mix;

            m_pos = (m_pos + 1) % MAX_DELAY;
        }
    }

private:
    std::array<float, MAX_DELAY> m_buffer_l{};
    std::array<float, MAX_DELAY> m_buffer_r{};
    size_t m_pos = 0;
};

}
