#include "sample_voice.h"
#include <cmath>

namespace disgrace_ns
{

    disgrace_ns::SampleVoice::SampleVoice(disgrace_ns::SampleData* data,
                             double engine_rate)
    : m_sample(data),
    m_engine_rate(engine_rate)
    {
        m_env.set(0.005f, 0.05f, 0.9f, 0.2f); // Moved to constructor body
    }

    void disgrace_ns::SampleVoice::start(uint8_t note,
                            uint8_t velocity,
                            float freq,
                            size_t offset_samples)
    {
        m_position = (double)offset_samples;

        double base_freq =
        440.0; // assume sample tuned to A4

        m_increment =
        (freq / base_freq) *
        (double(m_sample->sample_rate) /
        m_engine_rate);

        m_volume = velocity / 127.f;
         m_env.note_on();
        m_active = true;
    }

    void disgrace_ns::SampleVoice::stop()
    {
         m_env.note_off();
    }

    void disgrace_ns::SampleVoice::panic()
    {
        m_env.reset();
        m_active = false;
    }

    void disgrace_ns::SampleVoice::set_pitch(float freq)
    {
        double base_freq = 440.0;

        m_increment =
        (freq / base_freq) *
        (double(m_sample->sample_rate) /
        m_engine_rate);
    }

    void disgrace_ns::SampleVoice::set_volume(float vol)
    {
        m_volume = vol;
    }

    bool disgrace_ns::SampleVoice::active() const
    {
        return m_active;
    }


    void disgrace_ns::SampleVoice::process(float* out_l,
                              float* out_r,
                              size_t frames)
    {
        if (!m_env.active() || !m_sample)
        {
            m_active = false;
            return;
        }

        const size_t sample_size =
        m_sample->left.size();

        for (size_t i = 0; i < frames; ++i)
        {
            if (m_position >= sample_size - 1)
            {
                m_active = false;
                return;
            }

            size_t idx = size_t(m_position);
            float frac = float(m_position - idx);

            // Linear interpolation
            float l0 = m_sample->left[idx];
            float l1 = m_sample->left[idx + 1];
            float l  = l0 + (l1 - l0) * frac;

            float r = l;
            if (!m_sample->right.empty())
            {
                float r0 = m_sample->right[idx];
                float r1 = m_sample->right[idx + 1];
                r = r0 + (r1 - r0) * frac;
            }

            float env = m_env.process();

            out_l[i] += l * m_volume * env;
            out_r[i] += r * m_volume * env;

            m_position += m_increment;
        }

        if (!m_env.active())
            m_active = false;
    }



} // namespace disgrace_ns
