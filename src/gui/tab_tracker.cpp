#include "main_window.h"
#include "tracker_panel.h"

namespace disgrace_ns {

void MainWindow::init_tracker_tab(int w, int h) {
    m_tracker_tab = new Fl_Group(0, 65, w, h - 100, "Tracker");
    m_tracker_tab->begin();
    m_tracker_panel = new TrackerPanel(m_tracker_tab->x(), m_tracker_tab->y(), m_tracker_tab->w(), m_tracker_tab->h(), m_engine);
    m_tracker_tab->resizable(m_tracker_panel);
    m_tracker_tab->end();
}

} // namespace disgrace_ns
