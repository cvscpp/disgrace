#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>
#include "waveform_view.h"

namespace disgrace_ns {

class Engine;
class DetachedWindow;

class InstrumentPanel : public Fl_Group {
public:
    InstrumentPanel(int x, int y, int w, int h, Engine& engine);

    void update_instrument_list();
    void update_editor();

private:
    Engine& m_engine;
    int m_selected_instrument = -1;
    int m_selected_sample = -1;
    
    Fl_Group* m_left_panel;
    Fl_Group* m_right_panel;

    Fl_Button* m_new_btn;
    Fl_Button* m_load_btn;
    Fl_Button* m_save_btn;
    Fl_Button* m_delete_btn;
    Fl_File_Browser* m_file_browser;

    Fl_Scroll* m_inst_scroll;
    Fl_Group* m_inst_container;

    // Sampler Editor members
    Fl_Group* m_sampler_editor;
    Fl_Group* m_sampler_list_grp;
    Fl_Group* m_sampler_rec_grp;
    Fl_Scroll* m_sample_scroll;
    Fl_Group* m_sample_container;
    Fl_Button* m_add_sample_btn;
    Fl_Button* m_rec_btn;
    Fl_Check_Button* m_mono_btn;
    Fl_Choice* m_rec_input_ch;
    WaveformView* m_waveform_view;

    Fl_Button* m_detach_btn;
    DetachedWindow* m_detached_window = nullptr;

    void update_rec_inputs();

    static void cb_new(Fl_Widget*, void*);
    static void cb_load(Fl_Widget*, void*);
    static void cb_save(Fl_Widget*, void*);
    static void cb_delete(Fl_Widget*, void*);
    static void cb_inst_select(Fl_Widget*, void*);
    static void cb_inst_name(Fl_Widget*, void*);
    static void cb_inst_type(Fl_Widget*, void*);
    static void cb_detach(Fl_Widget*, void*);

    // Sample callbacks
    static void cb_add_sample(Fl_Widget*, void*);
    static void cb_load_sample(Fl_Widget*, void*);
    static void cb_remove_sample(Fl_Widget*, void*);
    static void cb_move_sample_up(Fl_Widget*, void*);
    static void cb_move_sample_down(Fl_Widget*, void*);
    static void cb_save_sample(Fl_Widget*, void*);
    static void cb_sample_name(Fl_Widget*, void*);
    static void cb_sample_select(Fl_Widget*, void*);
    static void cb_record_sample(Fl_Widget*, void*);
    static void cb_mono_toggle(Fl_Widget*, void*);
};

} // namespace disgrace_ns
