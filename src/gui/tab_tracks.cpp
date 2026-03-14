#include "main_window.h"
#include "tracks_panel.h"

namespace disgrace_ns {

void MainWindow::init_tracks_tab(int w, int h) {
    m_tracks_tab = new Fl_Group(0, 65, w, h - 100, "Tracks");
    m_tracks_panel = new TracksPanel(m_tracks_tab->x(), m_tracks_tab->y(), m_tracks_tab->w(), m_tracks_tab->h(), m_engine);
    m_tracks_tab->end();
}

} // namespace disgrace_ns
