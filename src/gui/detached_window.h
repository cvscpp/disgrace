#pragma once

#include <FL/Fl_Double_Window.H>

namespace disgrace_ns {

class TrackerPanel; // Forward declaration

class DetachedWindow : public Fl_Double_Window {
public:
    DetachedWindow(int w, int h, const char* title, Fl_Group* panel, Fl_Group* original_parent);

private:
    Fl_Group* m_panel;
    Fl_Group* m_original_parent;

    static void close_cb(Fl_Widget*, void*);
    void handle_close();
};

} // namespace disgrace_ns
