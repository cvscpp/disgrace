#include "sample_instrument.h"
#include <cmath>
#include <algorithm>

namespace disgrace_ns
{

    disgrace_ns::SampleInstrument::SampleInstrument(double engine_rate)
    : m_engine_rate(engine_rate)
    {
    }

    void disgrace_ns::SampleInstrument::note_on(uint8_t note, uint8_t velocity)
    {
        if (m_samples.empty()) return;
        
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

    void disgrace_ns::SampleInstrument::add_sample(const std::string& name, std::shared_ptr<disgrace_ns::SampleData> data)
    {
        m_samples.push_back({name, data});
    }

    void disgrace_ns::SampleInstrument::remove_sample(size_t index)
    {
        if (index < m_samples.size()) {
            m_samples.erase(m_samples.begin() + index);
        }
    }

    void disgrace_ns::SampleInstrument::move_sample(size_t from, size_t to)
    {
        if (from < m_samples.size() && to < m_samples.size() && from != to) {
            auto it_from = m_samples.begin() + from;
            SampleEntry entry = std::move(*it_from);
            m_samples.erase(it_from);
            m_samples.insert(m_samples.begin() + to, std::move(entry));
        }
    }

    void disgrace_ns::SampleInstrument::set_sample_name(size_t index, const std::string& name)
    {
        if (index < m_samples.size()) {
            m_samples[index].name = name;
        }
    }

    ::std::unique_ptr<disgrace_ns::Voice>
    disgrace_ns::SampleInstrument::create_voice()
    {
        if (m_samples.empty() || !m_samples[0].data) return nullptr;
        // For now, always use the first sample
        return ::std::make_unique<disgrace_ns::SampleVoice>(
            m_samples[0].data.get(),
            m_engine_rate);
    }

} // namespace disgrace_ns
