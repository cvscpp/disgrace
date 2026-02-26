#include "timing.h"

namespace disgrace_ns
{

    void disgrace_ns::Timing::set_sample_rate(uint32_t sr)
    {
        m_sample_rate = sr;
    }

    void disgrace_ns::Timing::set_lpb(uint32_t lpb)
    {
        // Placeholder, no actual implementation in the original code
        // m_lpb = lpb; if m_lpb member exists.
    }

    void disgrace_ns::Timing::set_bpm(int bpm)
    {
        if (bpm > 10 && bpm < 400)
            m_bpm = bpm;
    }

    void disgrace_ns::Timing::set_speed(int speed)
    {
        if (speed > 0 && speed < 32)
            m_speed = speed;
    }

    size_t disgrace_ns::Timing::samples_per_tick() const
    {
        double tick_sec = 2.5 / double(m_bpm);
        return static_cast<size_t>(m_sample_rate * tick_sec);
    }

    size_t disgrace_ns::Timing::samples_per_row() const
    {
        return samples_per_tick() * m_speed;
    }
    size_t disgrace_ns::Timing::samples_per_beat() const
    {
        return samples_per_row() * 4;
    }

    size_t disgrace_ns::Timing::samples_per_bar() const
    {
        return samples_per_beat() * 4;
    }

} // namespace disgrace_ns
