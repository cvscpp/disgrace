#pragma once
#include <cstddef>
#include <cstdint>

class Metronome
{
public:
    void set_sample_rate(double sr);
    void set_volume(float v);
    void reset();

    void process(float* out_l,
                 float* out_r,
                 size_t nframes,
                 size_t& samples_until_next_beat,
                 size_t samples_per_beat);

private:
    double m_sample_rate{44100.0};
    float m_volume{0.4f};

    size_t m_beat_counter{0};
    size_t m_click_remaining{0};

    float m_phase{0.f};
};
