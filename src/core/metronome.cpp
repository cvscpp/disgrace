#include "metronome.h"
#include <cmath>

namespace disgrace_ns
{

constexpr float PI_F = 3.14159265358979323846f; // Renamed from M_PI

void Metronome::set_sample_rate(double sr)
{
    m_sample_rate = sr;
}

void Metronome::set_volume(float v)
{
    m_volume = v;
}

void Metronome::reset()
{
    m_beat_counter = 0;
    m_click_remaining = 0;
    m_phase = 0.f;
}

void Metronome::process(float* out_l,
                        float* out_r,
                        size_t nframes,
                        size_t& samples_until_next_beat,
                        size_t samples_per_beat)
{
    const float freq_accent = 1200.f;
    const float freq_normal = 800.f;

    for (size_t i = 0; i < nframes; ++i)
    {
        if (samples_until_next_beat == 0)
        {
            m_click_remaining =
            static_cast<size_t>(0.02 * m_sample_rate); // 20ms

            m_beat_counter++;

            samples_until_next_beat =
            samples_per_beat;
        }

        float sample = 0.f;

        if (m_click_remaining > 0)
        {
            float freq =
            (m_beat_counter % 4 == 1)
            ? freq_accent
            : freq_normal;

            m_phase +=
            2.f * PI_F * freq / // Use PI_F
            m_sample_rate;

            sample =
            ::std::sin(m_phase) * m_volume;

            m_click_remaining--;
        }

        out_l[i] += sample;
        out_r[i] += sample;

        samples_until_next_beat--;
    }
}

} // namespace disgrace_ns
