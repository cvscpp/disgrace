#pragma once
#include <cstdint>

namespace disgrace_ns
{

enum class EffectType : uint8_t
{
    None = 0,
    SetBPM,
    SetSpeed,
    SetVolume,
    VolumeSlide,
    SetPanning,
    Portamento,
    Arpeggio,
    Vibrato,
    PitchSlide,
    NoteCut,
    Retrigger,
    SampleOffset,
    PatternBreak,
    Jump
};

struct TrackEvent
{
    uint8_t note = 255;      // 0-119, 255 = empty
    uint8_t sample_idx = 0;  // 0 = none/default
    uint8_t volume = 255;    // 255 = empty/default, 0-127 = actual
    
    // Each track row has two effect columns
    EffectType effect1 = EffectType::None;
    uint8_t param1 = 0;
    EffectType effect2 = EffectType::None;
    uint8_t param2 = 0;
};

} // namespace disgrace_ns
