#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Button.H>

namespace disgrace_ns {

class Engine;
class DetachedWindow;

class InstrumentPanel : public Fl_Group {
public:
    InstrumentPanel(int x, int y, int w, int h, Engine& engine);

private:
    Engine& m_engine;
    Fl_Browser* m_inst_list;
    Fl_Button* m_load_btn;
    Fl_Button* m_detach_btn;
    DetachedWindow* m_detached_window = nullptr;

    static void cb_inst_list(Fl_Widget*, void*);
    static void cb_detach(Fl_Widget*, void*);
};

} // namespace disgrace_ns
