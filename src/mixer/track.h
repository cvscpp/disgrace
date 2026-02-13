#pragma once

#include "../dsp/dsp_chain.h"
#include <vector>

namespace dg
{

    struct TrackEffectState
    {
        // volume slide
        int vol_slide_up = 0;
        int vol_slide_down = 0;

        // portamento
        float porta_target = 0.f;
        float porta_speed = 0.f;
        bool  porta_active = false;

        // retrig
        int retrig_ticks = 0;
        int retrig_counter = 0;

        // note cut
        int note_cut_tick = -1;
    };

class Instrument;

class Track
{
public:
    Track();

    void process(float* out_l,
                 float* out_r,
                 size_t nframes);

    void set_instrument(Instrument* inst);

    TrackEffectState m_fx_state;
    float m_volume = 1.0f;
    float m_frequency = 440.0f;
    float pan    = 0.0f;  // -1 left, +1 right
    uint8_t last_volume = 127;

    void note_on(uint8_t note, uint8_t velocity);
    void note_off();

    size_t Track::total_latency();

    void set_volume(float v);
    float volume() const;

    void set_mute(bool m);
    bool muted() const;

private:
    float m_volume = 1.f;
    bool  m_mute   = false;

    Instrument* m_instrument = nullptr;
    DSPChain    m_chain;
};

}
