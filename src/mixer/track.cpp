#include "track.h"
#include "../instrument/instrument.h"
#include <cmath> // Added for powf and fabs

namespace disgrace_ns
{

disgrace_ns::Track::Track()
    : m_meter(0.0f), m_current_freq(440.0f) // Initialize m_meter here
{
}

void disgrace_ns::Track::set_instrument(disgrace_ns::Instrument* inst)
{
    m_instrument = inst;
}

void disgrace_ns::Track::process(float* out_l,
                    float* out_r,
                    size_t nframes)
{
    if (m_instrument)
        m_instrument->process(out_l, out_r, nframes);

    m_chain.process(out_l, out_r, nframes);

    // apply volume + pan
    for (size_t i = 0; i < nframes; ++i)
    {
        float l = out_l[i];
        float r = out_r[i];

        float left_gain  = volume() * (pan <= 0 ? 1.0f : 1.0f - pan); // Corrected volume()
        float right_gain = volume() * (pan >= 0 ? 1.0f : 1.0f + pan); // Corrected volume()

        out_l[i] = l * left_gain;
        out_r[i] = r * right_gain;
    }

    float peak = 0.f;

    for (size_t i = 0; i < nframes; ++i)
    {
        float v = ::std::fabs(out_l[i]);
        if (v > peak) peak = v;

        v = ::std::fabs(out_r[i]);
        if (v > peak) peak = v;
    }

    // simple smoothing
    float prev = m_meter.load();
    float smoothed =
    0.8f * prev + 0.2f * peak;

    m_meter.store(smoothed);

}

float disgrace_ns::Track::note_to_frequency(uint8_t note) // Added namespace and class prefix
{
    return 440.0f * powf(2.0f, (int(note) - 69) / 12.0f);
}


void disgrace_ns::Track::note_on(uint8_t note, uint8_t velocity) // Added namespace and class prefix
{
    if (!m_instrument)
        return;

    float freq = note_to_frequency(note);

    if (m_fx_state.porta_active)
    {
        m_fx_state.porta_target = freq;
    }
    else
    {
        m_instrument->set_pitch(freq);
        m_instrument->note_on(note, velocity);
        m_current_freq = freq; // Update m_current_freq on note_on
    }
}

void disgrace_ns::Track::note_off()
{
    if (m_instrument)
        m_instrument->note_off();
}

size_t disgrace_ns::Track::total_latency() const
{
    size_t sum = 0;

    for (const auto& fx : m_chain.effects()) // Corrected m_fx_chain to m_chain
        sum += fx->latency();

    return sum;
}

void disgrace_ns::Track::set_volume(float v)
{
    m_volume = v;
}

float disgrace_ns::Track::volume() const
{
    return m_volume;
}

void disgrace_ns::Track::set_mute(bool m)
{
    m_mute = m;
}

bool disgrace_ns::Track::muted() const
{
    return m_mute;
}


void disgrace_ns::Track::process_tick(uint32_t engine_current_tick) // Added namespace and class prefix, updated parameter
{
    // --- volume slide ---
    if (m_fx_state.vol_slide_up > 0)
        m_volume += m_fx_state.vol_slide_up * 0.01f;

    if (m_fx_state.vol_slide_down > 0)
        m_volume -= m_fx_state.vol_slide_down * 0.01f;

    if (m_volume < 0.f) m_volume = 0.f;
    if (m_volume > 1.f) m_volume = 1.f;

    // --- portamento ---
    if (m_fx_state.porta_active)
    {
        float current = m_current_freq;

        if (current < m_fx_state.porta_target)
        {
            current += m_fx_state.porta_speed;
            if (current > m_fx_state.porta_target)
                current = m_fx_state.porta_target;
        }
        else
        {
            current -= m_fx_state.porta_speed;
            if (current < m_fx_state.porta_target)
                current = m_fx_state.porta_target;
        }

        m_current_freq = current;
        m_instrument->set_pitch(current);
    }


    // --- retrig ---
    if (m_fx_state.retrig_ticks > 0)
    {
        m_fx_state.retrig_counter++;

        if (m_fx_state.retrig_counter >=
            m_fx_state.retrig_ticks)
        {
            retrigger_note();
            m_fx_state.retrig_counter = 0;
        }
    }

    // --- note cut ---
    if (m_fx_state.note_cut_tick >= 0)
    {
        if (engine_current_tick == // Corrected from m_engine_current_tick
            m_fx_state.note_cut_tick)
        {
            note_off();
            m_fx_state.note_cut_tick = -1;
        }
    }
}

void disgrace_ns::Track::retrigger_note()
{
    if (m_instrument)
    {
        // Re-trigger the current note, assuming velocity of 100
        // A more robust solution might store the last note/velocity
        m_instrument->note_on(0, 100); // Placeholder, ideally should re-trigger last note
        m_instrument->set_pitch(m_current_freq);
    }
}

float disgrace_ns::Track::meter_level() const
{
    return m_meter.load();
}

bool disgrace_ns::Track::solo() const
{
    return m_solo;
}

void disgrace_ns::Track::set_solo(bool s)
{
    m_solo = s;
}

} // namespace disgrace_ns
