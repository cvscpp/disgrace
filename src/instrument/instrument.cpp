#include "instrument.h"

namespace dg
{

    Voice* Instrument::allocate_voice()
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

    void Instrument::note_on(uint8_t note,
                             uint8_t velocity)
    {
        float freq =
        440.0f *
        powf(2.0f,
             (int(note) - 69) / 12.0f);

        Voice* v = allocate_voice();
        v->start(note, velocity, freq);
    }

    void Instrument::note_off()
    {
        for (auto& v : m_voices)
        {
            if (v && v->active())
                v->stop();
        }
    }

    void Instrument::set_pitch(float freq)
    {
        for (auto& v : m_voices)
            if (v && v->active())
                v->set_pitch(freq);
    }

    void Instrument::set_volume(float vol)
    {
        for (auto& v : m_voices)
            if (v && v->active())
                v->set_volume(vol);
    }

    void Instrument::process(float* out_l,
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

}
