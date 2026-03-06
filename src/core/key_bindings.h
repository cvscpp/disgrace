#pragma once

#include <string>
#include <map>
#include <FL/Fl.H>

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
    NoteC3
};

struct KeyCombo {
    int key;
    int modifiers; // FL_CTRL, FL_SHIFT, etc.

    bool operator<(const KeyCombo& other) const {
        if (key != other.key) return key < other.key;
        return modifiers < other.modifiers;
    }
};

class KeyBindings {
public:
    KeyBindings();

    void set_defaults();
    Action get_action(int key, int modifiers) const;
    std::string get_action_name(Action action) const;
    std::string get_key_name(Action action) const;
    void assign(Action action, int key, int modifiers);

private:
    std::map<Action, KeyCombo> m_action_to_key;
    std::map<KeyCombo, Action> m_key_to_action;
};

} // namespace disgrace_ns
