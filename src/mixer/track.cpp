#include "track.h"
#include "../instrument/instrument.h"

namespace dg
{

Track::Track()
{
}

void Track::set_instrument(Instrument* inst)
{
    m_instrument = inst;
}

void Track::process(float* out_l,
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

        float left_gain  = volume * (pan <= 0 ? 1.0f : 1.0f - pan);
        float right_gain = volume * (pan >= 0 ? 1.0f : 1.0f + pan);

        out_l[i] = l * left_gain;
        out_r[i] = r * right_gain;
    }
}

float Track::note_to_frequency(uint8_t note)
{
    return 440.0f * powf(2.0f, (int(note) - 69) / 12.0f);
}


void Track::note_on(uint8_t note,
                    uint8_t velocity)
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
    }
}

void Track::note_off()
{
    if (m_instrument)
        m_instrument->note_off();
}

size_t Track::total_latency() const
{
    size_t sum = 0;

    for (const auto& fx : m_fx_chain.effects())
        sum += fx->latency();

    return sum;
}

void Track::set_volume(float v)
{
    m_volume = v;
}

float Track::volume() const
{
    return m_volume;
}

void Track::set_mute(bool m)
{
    m_mute = m;
}

bool Track::muted() const
{
    return m_mute;
}


void Track::process_tick()
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
        if (m_engine_current_tick ==
            m_fx_state.note_cut_tick)
        {
            note_off();
            m_fx_state.note_cut_tick = -1;
        }
    }

    if (m_mute)
    {
        std::fill(out_l, out_l + frames, 0.f);
        std::fill(out_r, out_r + frames, 0.f);
        return;
    }

    for (size_t i = 0; i < frames; ++i)
    {
        out_l[i] *= m_volume;
        out_r[i] *= m_volume;
    }

}


}
