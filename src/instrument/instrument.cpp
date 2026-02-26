#include "../core/engine.h" // ADDED
#include <cmath> // Added for powf
#include "instrument.h"

namespace disgrace_ns
{

    disgrace_ns::Voice* disgrace_ns::Instrument::allocate_voice()
    {
        // find inactive voice
        for (auto& v : m_voices)
        {
            if (!v)
                v = create_voice();

            if (!v->active())
                return v.get();
        }

        // voice stealing: steal oldest (voice 0)
        return m_voices[0].get();
    }

    void disgrace_ns::Instrument::note_on(uint8_t note,
                             uint8_t velocity)
    {
        float freq =
        440.0f *
        powf(2.0f,
             (int(note) - 69) / 12.0f);

        disgrace_ns::Voice* v = allocate_voice();
        v->start(note, velocity, freq);
    }

    void disgrace_ns::Instrument::note_off()
    {
        for (auto& v : m_voices)
        {
            if (v && v->active())
                v->stop();
        }
    }

    void disgrace_ns::Instrument::set_pitch(float freq)
    {
        for (auto& v : m_voices)
            if (v && v->active())
                v->set_pitch(freq);
    }

    void disgrace_ns::Instrument::set_volume(float vol)
    {
        for (auto& v : m_voices)
            if (v && v->active())
                v->set_volume(vol);
    }

    void disgrace_ns::Instrument::process(float* out_l,
                             float* out_r,
                             size_t frames)
    {
        for (size_t i = 0; i < frames; ++i)
        {
            out_l[i] = 0.f;
            out_r[i] = 0.f;
        }

        for (auto& v : m_voices)
        {
            if (v && v->active())
                v->process(out_l, out_r, frames);
        }
    }

} // namespace disgrace_ns
