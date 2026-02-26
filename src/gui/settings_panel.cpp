#include "settings_panel.h"
#include "../core/engine.h"
#include <FL/Fl_Button.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>

namespace disgrace_ns {

// Helper function to recursively apply settings to all widgets
void apply_to_all_widgets(Fl_Widget* w, int font_size, int btn_h) {
    w->labelsize(font_size);
    
    // If it's a button, try to resize it
    if (dynamic_cast<Fl_Button*>(w)) {
        w->size(w->w(), btn_h); // Only height for now as width depends on layout context
    }

    // If it's a group, recurse into children
    Fl_Group* g = w->as_group();
    if (g) {
        for (int i = 0; i < g->children(); i++) {
            apply_to_all_widgets(g->child(i), font_size, btn_h);
        }
    }
}

void apply_gui_settings(Engine& engine) {
    FL_NORMAL_SIZE = engine.m_gui_font_size;
    
    for (Fl_Window* win = Fl::first_window(); win; win = Fl::next_window(win)) {
        apply_to_all_widgets(win, engine.m_gui_font_size, engine.m_gui_button_height);
        win->redraw();
    }
}

SettingsPanel::SettingsPanel(int x, int y, int w, int h, Engine& engine)
    : Fl_Group(x, y, w, h), m_engine(engine) {
    
    begin();

    m_sub_tabs = new Fl_Tabs(x, y, w, h);
    
    init_audio_grp(x, y, w, h);
    init_midi_grp(x, y, w, h);
    init_mixer_grp(x, y, w, h);
    init_gui_grp(x, y, w, h);
    init_kbd_grp(x, y, w, h);
    init_misc_grp(x, y, w, h);

    m_sub_tabs->end();
    resizable(m_sub_tabs);

    end();
}

void SettingsPanel::init_audio_grp(int x, int y, int w, int h) {
    m_audio_grp = new Fl_Group(x, y + 25, w, h - 25, "Audio");
    m_audio_grp->begin();
    Fl_Box* title = new Fl_Box(x + 20, y + 40, 200, 25, "Audio Device: JACK");
    title->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    
    m_audio_ins = new Fl_Value_Input(x + 140, y + 70, 40, 25, "Input Channels:");
    m_audio_ins->value(2);
    m_audio_ins->minimum(1);
    m_audio_ins->maximum(64);
    m_audio_ins->step(1);
    m_audio_ins->align(FL_ALIGN_LEFT);
    
    m_audio_outs = new Fl_Value_Input(x + 140, y + 100, 40, 25, "Output Channels:");
    m_audio_outs->value(2);
    m_audio_outs->minimum(1);
    m_audio_outs->maximum(64);
    m_audio_outs->step(1);
    m_audio_outs->align(FL_ALIGN_LEFT);

    m_reinit_audio_btn = new Fl_Button(x + 20, y + 140, 160, 25, "Reinitialize Audio");
    m_reinit_audio_btn->callback(cb_reinit_audio, this);

    m_audio_grp->end();
}

void SettingsPanel::init_midi_grp(int x, int y, int w, int h) {
    m_midi_grp = new Fl_Group(x, y + 25, w, h - 25, "MIDI");
    m_midi_grp->begin();
    Fl_Box* title = new Fl_Box(x + 20, y + 40, 200, 25, "MIDI Config");
    title->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    m_midi_ins = new Fl_Value_Input(x + 140, y + 70, 40, 25, "MIDI Inputs:");
    m_midi_ins->value(1);
    m_midi_ins->minimum(0);
    m_midi_ins->maximum(16);
    m_midi_ins->step(1);
    m_midi_ins->align(FL_ALIGN_LEFT);

    m_midi_outs = new Fl_Value_Input(x + 140, y + 100, 40, 25, "MIDI Outputs:");
    m_midi_outs->value(1);
    m_midi_outs->minimum(0);
    m_midi_outs->maximum(16);
    m_midi_outs->step(1);
    m_midi_outs->align(FL_ALIGN_LEFT);

    m_reinit_midi_btn = new Fl_Button(x + 20, y + 140, 160, 25, "Reinitialize MIDI");
    m_reinit_midi_btn->callback(cb_reinit_midi, this);

    m_midi_grp->end();
}

void SettingsPanel::init_mixer_grp(int x, int y, int w, int h) {
    m_mixer_grp = new Fl_Group(x, y + 25, w, h - 25, "Mixer");
    m_mixer_grp->begin();
    new Fl_Box(x + 20, y + 50, 200, 25, "Default Track Count: 8");
    m_mixer_grp->end();
}

void SettingsPanel::init_gui_grp(int x, int y, int w, int h) {
    m_gui_grp = new Fl_Group(x, y + 25, w, h - 25, "GUI");
    m_gui_grp->begin();
    
    Fl_Box* title = new Fl_Box(x + 20, y + 40, 200, 25, "GUI Settings");
    title->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    m_gui_theme = new Fl_Choice(x + 120, y + 70, 100, 25, "Theme:");
    m_gui_theme->add("none");
    m_gui_theme->add("base");
    m_gui_theme->add("plastic");
    m_gui_theme->add("gtk+");
    m_gui_theme->add("gleam");
    m_gui_theme->value(0);
    m_gui_theme->callback(cb_gui_theme, this);
    m_gui_theme->align(FL_ALIGN_LEFT);

    m_gui_btn_h = new Fl_Choice(x + 120, y + 100, 100, 25, "Btn Height:");
    m_gui_btn_h->add("10");
    m_gui_btn_h->add("15");
    m_gui_btn_h->add("20");
    m_gui_btn_h->add("25");
    m_gui_btn_h->add("30");
    m_gui_btn_h->value(3); // Default 25
    m_gui_btn_h->callback(cb_gui_btn_h, this);
    m_gui_btn_h->align(FL_ALIGN_LEFT);

    m_gui_font_size = new Fl_Choice(x + 120, y + 130, 100, 25, "Font Size:");
    m_gui_font_size->add("8");
    m_gui_font_size->add("10");
    m_gui_font_size->add("12");
    m_gui_font_size->add("14");
    m_gui_font_size->add("16");
    m_gui_font_size->add("18");
    m_gui_font_size->add("20");
    m_gui_font_size->value(2); // Default 12
    m_gui_font_size->callback(cb_gui_font_size, this);
    m_gui_font_size->align(FL_ALIGN_LEFT);

    Fl_Check_Button* show_meters = new Fl_Check_Button(x + 20, y + 160, 150, 25, "Show Level Meters");
    show_meters->value(1);
    
    m_gui_grp->end();
}

void SettingsPanel::init_kbd_grp(int x, int y, int w, int h) {
    m_kbd_grp = new Fl_Group(x, y + 25, w, h - 25, "Keyboard");
    m_kbd_grp->begin();
    new Fl_Box(x + 20, y + 50, 200, 25, "Keybinding Preset: Default");
    m_kbd_grp->end();
}

void SettingsPanel::init_misc_grp(int x, int y, int w, int h) {
    m_misc_grp = new Fl_Group(x, y + 25, w, h - 25, "Misc");
    m_misc_grp->begin();
    new Fl_Box(x + 20, y + 50, 200, 25, "Auto-save: Enabled");
    m_misc_grp->end();
}

void SettingsPanel::cb_reinit_audio(Fl_Widget*, void* data) {
    auto* self = static_cast<SettingsPanel*>(data);
    self->m_engine.reinitialize_audio(
        static_cast<uint32_t>(self->m_audio_ins->value()),
        static_cast<uint32_t>(self->m_audio_outs->value()),
        static_cast<uint32_t>(self->m_midi_ins->value()),
        static_cast<uint32_t>(self->m_midi_outs->value())
    );
}

void SettingsPanel::cb_reinit_midi(Fl_Widget*, void* data) {
    auto* self = static_cast<SettingsPanel*>(data);
    self->m_engine.reinitialize_audio(
        static_cast<uint32_t>(self->m_audio_ins->value()),
        static_cast<uint32_t>(self->m_audio_outs->value()),
        static_cast<uint32_t>(self->m_midi_ins->value()),
        static_cast<uint32_t>(self->m_midi_outs->value())
    );
}

void SettingsPanel::cb_gui_theme(Fl_Widget* w, void* data) {
    auto* choice = static_cast<Fl_Choice*>(w);
    const char* theme = choice->text();
    if (theme) {
        Fl::scheme(theme);
        Fl::redraw();
    }
}

void SettingsPanel::cb_gui_btn_h(Fl_Widget* w, void* data) {
    auto* self = static_cast<SettingsPanel*>(data);
    auto* choice = static_cast<Fl_Choice*>(w);
    const char* val = choice->text();
    if (val) {
        self->m_engine.m_gui_button_height = atoi(val);
        apply_gui_settings(self->m_engine);
    }
}

void SettingsPanel::cb_gui_font_size(Fl_Widget* w, void* data) {
    auto* self = static_cast<SettingsPanel*>(data);
    auto* choice = static_cast<Fl_Choice*>(w);
    const char* val = choice->text();
    if (val) {
        self->m_engine.m_gui_font_size = atoi(val);
        apply_gui_settings(self->m_engine);
    }
}

} // namespace disgrace_ns
