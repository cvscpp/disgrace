#include "timing.h"

namespace dg
{

    void Timing::set_sample_rate(double sr)
    {
        m_sample_rate = sr;
    }

    void Timing::set_bpm(int bpm)
    {
        if (bpm > 10 && bpm < 400)
            m_bpm = bpm;
    }

    void Timing::set_speed(int speed)
    {
        if (speed > 0 && speed < 32)
            m_speed = speed;
    }

    size_t Timing::samples_per_tick() const
    {
        double tick_sec = 2.5 / double(m_bpm);
        return static_cast<size_t>(m_sample_rate * tick_sec);
    }

    size_t Timing::samples_per_row() const
    {
        return samples_per_tick() * m_speed;
    }

}
