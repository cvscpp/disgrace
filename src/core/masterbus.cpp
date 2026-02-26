#include "masterbus.h"
#include <cmath>

namespace disgrace_ns
{

void MasterBus::set_gain(float g)
{
    m_gain.store(g);
}

float MasterBus::gain() const
{
    return m_gain.load();
}

float MasterBus::soft_clip(float x)
{
    // simple musical soft limiter
    return ::std::tanh(x);
}

void MasterBus::process(float* l,
                        float* r,
                        size_t nframes)
{
    float gain = m_gain.load();
    float peak = 0.f;

    for (size_t i = 0; i < nframes; ++i)
    {
        float sl = l[i] * gain;
        float sr = r[i] * gain;

        sl = soft_clip(sl);
        sr = soft_clip(sr);

        l[i] = sl;
        r[i] = sr;

        float p = ::std::fabs(sl);
        if (p > peak) peak = p;

        p = ::std::fabs(sr);
        if (p > peak) peak = p;
    }

    float prev = m_meter.load();
    float smoothed =
    0.9f * prev + 0.1f * peak;

    m_meter.store(smoothed);
}

float MasterBus::meter() const
{
    return m_meter.load();
}

} // namespace disgrace_ns