#pragma once

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

#include <FL/Fl_Group.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Color_Chooser.H>

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
    Fl_Box* m_audio_status;

    Fl_Value_Input* m_midi_ins;
    Fl_Value_Input* m_midi_outs;
    Fl_Button* m_reinit_midi_btn;

    Fl_Choice* m_gui_theme;
    Fl_Choice* m_gui_btn_h;
    Fl_Choice* m_gui_font_size;
    Fl_Button* m_waveform_color_btn;
    Fl_Button* m_bg_color_btn;
    Fl_Button* m_fg_color_btn;
    Fl_Choice* m_boxtype_choice;
    Fl_Choice* m_btn_boxtype_choice;

    Fl_Choice* m_kbd_layout;
    Fl_Choice* m_action_choice;
    Fl_Button* m_assign_btn;
    Fl_Box* m_current_binding_box;

    void init_audio_grp(int x, int y, int w, int h);
    void init_midi_grp(int x, int y, int w, int h);
    void init_mixer_grp(int x, int y, int w, int h);
    void init_gui_grp(int x, int y, int w, int h);
    void init_kbd_grp(int x, int y, int w, int h);
    void init_misc_grp(int x, int y, int w, int h);

    void update_audio_status();
    void update_kbd_list();

    static void cb_reinit_audio(Fl_Widget*, void*);
    static void cb_reinit_midi(Fl_Widget*, void*);
    static void cb_gui_theme(Fl_Widget*, void*);
    static void cb_gui_btn_h(Fl_Widget*, void*);
    static void cb_gui_font_size(Fl_Widget*, void*);
    static void cb_waveform_color(Fl_Widget*, void*);
    static void cb_bg_color(Fl_Widget*, void*);
    static void cb_fg_color(Fl_Widget*, void*);
    static void cb_boxtype(Fl_Widget*, void*);
    static void cb_btn_boxtype(Fl_Widget*, void*);
    static void cb_kbd_layout(Fl_Widget*, void*);
    static void cb_assign_key(Fl_Widget*, void*);
};

} // namespace disgrace_ns
