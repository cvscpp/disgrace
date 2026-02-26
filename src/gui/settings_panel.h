#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Choice.H>

namespace disgrace_ns {

class Engine;

class SettingsPanel : public Fl_Group {
public:
    SettingsPanel(int x, int y, int w, int h, Engine& engine);

private:
    Engine& m_engine;
    Fl_Tabs* m_sub_tabs;
    
    Fl_Group* m_audio_grp;
    Fl_Group* m_midi_grp;
    Fl_Group* m_mixer_grp;
    Fl_Group* m_gui_grp;
    Fl_Group* m_kbd_grp;
    Fl_Group* m_misc_grp;

    Fl_Value_Input* m_audio_ins;
    Fl_Value_Input* m_audio_outs;
    Fl_Button* m_reinit_audio_btn;

    Fl_Value_Input* m_midi_ins;
    Fl_Value_Input* m_midi_outs;
    Fl_Button* m_reinit_midi_btn;

    Fl_Choice* m_gui_theme;
    Fl_Choice* m_gui_btn_h;
    Fl_Choice* m_gui_font_size;

    Fl_Choice* m_action_choice;
    Fl_Button* m_assign_btn;
    Fl_Box* m_current_binding_box;

    void init_audio_grp(int x, int y, int w, int h);
    void init_midi_grp(int x, int y, int w, int h);
    void init_mixer_grp(int x, int y, int w, int h);
    void init_gui_grp(int x, int y, int w, int h);
    void init_kbd_grp(int x, int y, int w, int h);
    void init_misc_grp(int x, int y, int w, int h);

    void update_kbd_list();

    static void cb_reinit_audio(Fl_Widget*, void*);
    static void cb_reinit_midi(Fl_Widget*, void*);
    static void cb_gui_theme(Fl_Widget*, void*);
    static void cb_gui_btn_h(Fl_Widget*, void*);
    static void cb_gui_font_size(Fl_Widget*, void*);
    static void cb_assign_key(Fl_Widget*, void*);
};

} // namespace disgrace_ns
