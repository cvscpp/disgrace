/*
 * Disgrace - Digital Audio Workstation
 * Copyright (C) 2025  Miroslav Shaltev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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

struct TrackEvent
{
    uint8_t note = 255;      // 0-119, 255 = empty
    uint8_t sample_idx = 0;  // 0 = none/default
    uint8_t volume = 255;    // 255 = empty/default, 0-127 = actual
    
    // Each track row has two effect columns
    uint8_t effect1 = 0;
    uint8_t param1 = 0;
    uint8_t effect2 = 0;
    uint8_t param2 = 0;
};

// Display octave for a note value. The engine uses raw MIDI note numbers
// everywhere (69 = A4/440 Hz) and note names follow the MIDI/Yamaha
// convention (60 = C-4), matching the piano labels and Renoise: the raw
// value divided by 12 reads one octave too high (60 would show "C-5").
// Values above the valid range clamp to the top octave, the bottom
// sub-octave (0-11) clamps to octave 0 ("C-0".."B-0").
inline int note_display_octave(uint8_t note)
{
    if (note > 119) return 9;
    int o = (int)note / 12 - 1;
    return o < 0 ? 0 : o;
}

} // namespace disgrace_ns
