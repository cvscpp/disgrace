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

// ---- Tracker note range ----------------------------------------------------
// A note is a raw MIDI note number (60 = C-4, the MIDI/Yamaha labelling also
// used by Renoise), so the tracker's ten octaves C-0..B-9 span 12..131 —
// the same range Renoise and other trackers expose through the Oct control.
// 254/255 are the note-off / empty sentinels, which still leaves the whole
// range in one byte.  Notes above 127 cannot travel over MIDI (128..131 are
// G#-9..B-9); instruments that speak MIDI clamp them via note_midi_clamp().
constexpr uint8_t NOTE_MIN      = 12;   // C-0
constexpr uint8_t NOTE_MAX      = 131;  // B-9
constexpr uint8_t MIDI_NOTE_MAX = 127;

constexpr int NOTE_MIN_OCTAVE = 0;      // octave shown for NOTE_MIN
constexpr int NOTE_MAX_OCTAVE = 9;      // octave shown for notes >= 120

constexpr uint8_t NOTE_OFF_VALUE   = 254;  // "==="
constexpr uint8_t NOTE_EMPTY_VALUE = 255;  // "---"

inline bool note_is_playable(uint8_t note)
{
    return note >= NOTE_MIN && note <= NOTE_MAX;
}

inline uint8_t note_clamp(int note)
{
    if (note < (int)NOTE_MIN) return NOTE_MIN;
    if (note > (int)NOTE_MAX) return NOTE_MAX;
    return (uint8_t)note;
}

// Highest note a MIDI-aware instrument can actually play.
inline uint8_t note_midi_clamp(uint8_t note)
{
    return note > MIDI_NOTE_MAX ? MIDI_NOTE_MAX : note;
}

struct TrackEvent
{
    uint8_t note = 255;      // NOTE_MIN..NOTE_MAX, 254 = note off, 255 = empty
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
// The playable range therefore runs C-0 (12) to B-9 (131).  Values above
// it clamp to the top octave; the sub-octave (0-11, below C-0) clamps to
// octave 0 because tracker notation has no negative octaves.
inline int note_display_octave(uint8_t note)
{
    if (note > NOTE_MAX) return NOTE_MAX_OCTAVE;
    int o = (int)note / 12 - 1;
    return o < NOTE_MIN_OCTAVE ? NOTE_MIN_OCTAVE : o;
}

} // namespace disgrace_ns
