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

void MasterBus::set_mute(bool m)
{
    m_muted.store(m);
}

bool MasterBus::muted() const
{
    return m_muted.load();
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
    m_chain.process(l, r, nframes);

    float gain = m_muted.load() ? 0.f : m_gain.load();
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

        if (m_is_recording.load()) {
            m_recorded_l.push_back(sl);
            m_recorded_r.push_back(sr);
        }

        float pl = ::std::fabs(sl);
        if (pl > peak_l) peak_l = pl;

        float pr = ::std::fabs(sr);
        if (pr > peak_r) peak_r = pr;
    }

    if (m_export_mute.load()) {
        for (size_t i = 0; i < nframes; ++i) {
            l[i] = 0.f; r[i] = 0.f;
        }
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

void MasterBus::set_effect(size_t index, ::std::unique_ptr<disgrace_ns::DSP> dsp)
{
    m_chain.set(index, std::move(dsp));
}

void MasterBus::enable_effect(size_t index, bool en)
{
    m_chain.enable(index, en);
}

void MasterBus::move_effect_up(size_t index)
{
    m_chain.move_up(index);
}

void MasterBus::move_effect_down(size_t index)
{
    m_chain.move_down(index);
}

void MasterBus::remove_effect(size_t index)
{
    m_chain.remove(index);
}

void MasterBus::load_effect_chain(const std::string& path)
{
    m_chain.load_chain(path);
}

void MasterBus::save_effect_chain(const std::string& path)
{
    m_chain.save_chain(path);
}

disgrace_ns::DSP* MasterBus::get_effect(size_t index) const
{
    if (index >= disgrace_ns::MAX_INSERTS) return nullptr;
    return m_chain.effects()[index].get();
}

} // namespace disgrace_ns