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

#include <string>
#include <map>
#include <vector>

namespace disgrace_ns {

enum class Action {
    Play,
    PlaySong,
    PlayPattern,
    PlayFromPosition,
    Stop,
    Record,
    ToggleMetronome,
    Undo,
    Redo,
    Copy,
    Cut,
    Paste,
    Clear,
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    NoteOff,
    NoteC,
    NoteCs,
    NoteD,
    NoteDs,
    NoteE,
    NoteF,
    NoteFs,
    NoteG,
    NoteGs,
    NoteA,
    NoteAs,
    NoteB,
    NoteC2,
    NoteCs2,
    NoteD2,
    NoteDs2,
    NoteE2,
    NoteF2,
    NoteFs2,
    NoteG2,
    NoteGs2,
    NoteA2,
    NoteAs2,
    NoteB2,
    NoteC3,
    // Third octave upward: the keys continuing after I/O/P on the upper row.
    NoteCs3,
    NoteD3,
    NoteDs3,
    NoteE3,
    NoteF3,
    NextPattern,
    PrevPattern,
    OctaveUp,
    OctaveDown,
    NextOrderPos,
    PrevOrderPos,
    JumpToRow0,
    JumpToRow16,
    JumpToRow32,
    JumpToRow48,
    InsertRow,
    DeleteRow,
    InsertPattern,
    DeletePattern,
    DuplicatePattern,
    SelectAll,
    MuteTrack,
    SoloTrack,
    JumpToNextColumn,
    JumpToPrevColumn,
    IncPatternIndex,
    DecPatternIndex
};

enum class KeyboardLayout {
    Auto,
    QWERTY,
    QWERTZ
};

struct KeyCombo {
    int key;
    int modifiers; // FL_CTRL, FL_SHIFT, etc.

    bool operator<(const KeyCombo& other) const {
        if (key != other.key) return key < other.key;
        return modifiers < other.modifiers;
    }
};

// wxGTK reports printable non-ASCII keys (e.g. U-umlaut on a German
// keyboard) as key code 0 and only exposes the character through the
// Unicode value, which is what note bindings are stored as.  Callers pass
// wxKeyEvent::GetKeyCode() and ::GetUnicodeKey() so this header stays
// independent of wxWidgets.
inline int normalize_key_for_binding(int key_code, int unicode_key)
{
    if (key_code == 0 && unicode_key > 0) return unicode_key;
    return key_code;
}

class KeyBindings {
public:
    KeyBindings();
    void set_defaults();
    void set_layout(KeyboardLayout layout);
    KeyboardLayout get_layout() const { return m_layout; }
    KeyboardLayout detect_layout();

    Action get_action(int key, int modifiers) const;
    std::string get_action_name(Action action) const;
    std::string get_key_name(Action action) const;
    void assign(Action action, int key, int modifiers);

    // Returns all known actions in display order (for building shortcut lists)
    static const std::vector<Action>& all_actions();

    // Semitone offset above the base octave's C that a note action plays
    // (0 = C of the lower row, 12 = C of the upper row, 25..29 = the keys
    // continuing past I/O/P), or -1 when the action is not a pitch.
    // Note-Off is not a pitch and reports -1 as well.
    static int note_semitone(Action action);

private:
    std::map<Action, KeyCombo> m_action_to_key;
    std::map<KeyCombo, Action> m_key_to_action;
    KeyboardLayout m_layout = KeyboardLayout::Auto;
};

} // namespace disgrace_ns
