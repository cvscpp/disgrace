#pragma once
#include <cstdint>

namespace disgrace_ns
{

enum class EffectType : uint8_t
{
    None,
    SetTempo,
    Volume,
    PatternBreak,
    Jump,
    NoteCut
};

struct NoteEvent
{
    uint8_t note = 255;      // 0-119, 255 = empty
    uint8_t instrument = 0;
    uint8_t volume = 0;      // optional
    uint8_t effect = 0;      // effect command
    uint8_t param = 0;       // effect parameter
};

} // namespace disgrace_ns
