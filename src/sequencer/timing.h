#pragma once

#include <cstdint>
#include <cstddef> // Added for size_t

namespace disgrace_ns
{

class Timing
{
public:
    void set_sample_rate(uint32_t sr);
    void set_bpm(int bpm); // Renamed from set_tempo
    double tempo() const { return m_bpm; } // Added tempo()
    void set_lpb(uint32_t lpb);
    uint32_t lpb() const { return m_speed; } // Added this line
    void set_speed(int speed);

    int bpm() const { return m_bpm; }
    int speed() const { return m_speed; }

    size_t samples_per_tick() const;
    size_t samples_per_row() const;

    size_t samples_per_beat() const;
    size_t samples_per_bar() const;


private:
    double m_sample_rate = 48000.0;
    int m_bpm = 125;     // default tracker tempo
    int m_speed = 6;     // ticks per row
};

} // namespace disgrace_ns
