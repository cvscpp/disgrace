#include "detached_window.h"
#include <FL/Fl_Group.H>

namespace disgrace_ns {

DetachedWindow::DetachedWindow(int w, int h, const char* title, Fl_Group* panel, Fl_Group* original_parent)
    : Fl_Double_Window(w, h, title), m_panel(panel), m_original_parent(original_parent) {
    callback(close_cb, this);
    m_panel->position(0, 0);
    m_panel->size(w, h);
    add(m_panel);
    resizable(m_panel);
}

void DetachedWindow::close_cb(Fl_Widget* w, void* data) {
    static_cast<DetachedWindow*>(data)->handle_close();
}

void DetachedWindow::handle_close() {
    m_original_parent->add(m_panel);
    m_panel->position(0, 0); // Reset position relative to its new parent
    m_panel->size(m_original_parent->w(), m_original_parent->h()); // Resize to fill parent
    m_panel->show();
    hide();
}

} // namespace disgrace_ns
