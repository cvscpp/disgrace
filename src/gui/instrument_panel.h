#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Browser.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Choice.H>

namespace disgrace_ns {

class Engine;
class DetachedWindow;

class InstrumentPanel : public Fl_Group {
public:
    InstrumentPanel(int x, int y, int w, int h, Engine& engine);

    void update_instrument_list();

private:
    Engine& m_engine;
    
    Fl_Group* m_left_panel;
    Fl_Group* m_right_panel;

    Fl_Button* m_new_btn;
    Fl_Button* m_load_btn;
    Fl_Button* m_save_btn;
    Fl_Button* m_delete_btn;
    Fl_File_Browser* m_file_browser;

    Fl_Scroll* m_inst_scroll;
    Fl_Group* m_inst_container;

    Fl_Button* m_detach_btn;
    DetachedWindow* m_detached_window = nullptr;

    static void cb_new(Fl_Widget*, void*);
    static void cb_load(Fl_Widget*, void*);
    static void cb_save(Fl_Widget*, void*);
    static void cb_delete(Fl_Widget*, void*);
    static void cb_inst_name(Fl_Widget*, void*);
    static void cb_inst_type(Fl_Widget*, void*);
    static void cb_detach(Fl_Widget*, void*);
};

} // namespace disgrace_ns
