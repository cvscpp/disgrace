#include "sample_instrument.h"
#include <cmath>

namespace disgrace_ns
{

    disgrace_ns::SampleInstrument::SampleInstrument(
        disgrace_ns::SampleData* data,
        double engine_rate)
    : m_sample(data),
    m_engine_rate(engine_rate)
    {
    }

    void disgrace_ns::SampleInstrument::note_on(uint8_t note, uint8_t velocity)
    {
        float freq = 440.0f * powf(2.0f, (int(note) - 69) / 12.0f);
        disgrace_ns::Voice* v = allocate_voice();
        if (v) v->start(note, velocity, freq);
    }

    void disgrace_ns::SampleInstrument::note_off()
    {
        for (auto& v : m_voices) if (v && v->active()) v->stop();
    }

    void disgrace_ns::SampleInstrument::set_volume(float vol)
    {
        for (auto& v : m_voices) if (v && v->active()) v->set_volume(vol);
    }

    void disgrace_ns::SampleInstrument::set_pitch(float freq)
    {
        for (auto& v : m_voices) if (v && v->active()) v->set_pitch(freq);
    }

    void disgrace_ns::SampleInstrument::process(float* out_l, float* out_r, size_t frames)
    {
        for (size_t i = 0; i < frames; ++i) { out_l[i] = 0.f; out_r[i] = 0.f; }
        for (auto& v : m_voices) if (v && v->active()) v->process(out_l, out_r, frames);
    }

    ::std::unique_ptr<disgrace_ns::Voice>
    disgrace_ns::SampleInstrument::create_voice()
    {
        return ::std::make_unique<disgrace_ns::SampleVoice>(
            m_sample,
            m_engine_rate);
    }

} // namespace disgrace_ns
