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
    float peak_l = 0.f;
    float peak_r = 0.f;

    for (size_t i = 0; i < nframes; ++i)
    {
        float sl = l[i] * gain;
        float sr = r[i] * gain;

        sl = soft_clip(sl);
        sr = soft_clip(sr);

        l[i] = sl;
        r[i] = sr;

        float pl = ::std::fabs(sl);
        if (pl > peak_l) peak_l = pl;

        float pr = ::std::fabs(sr);
        if (pr > peak_r) peak_r = pr;
    }

    float prev_l = m_meter_l.load();
    float smoothed_l = 0.9f * prev_l + 0.1f * peak_l;
    m_meter_l.store(smoothed_l);

    float prev_r = m_meter_r.load();
    float smoothed_r = 0.9f * prev_r + 0.1f * peak_r;
    m_meter_r.store(smoothed_r);
}

float MasterBus::meter_l() const
{
    return m_meter_l.load();
}

float MasterBus::meter_r() const
{
    return m_meter_r.load();
}

} // namespace disgrace_ns