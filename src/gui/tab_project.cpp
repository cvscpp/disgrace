#include "main_window.h"
#include "project_panel.h"

namespace disgrace_ns {

void MainWindow::init_project_tab(int w, int h) {
    m_project_tab = new Fl_Group(0, 65, w, h - 100, "Project");
    m_project_panel = new ProjectPanel(m_project_tab->x(), m_project_tab->y(), m_project_tab->w(), m_project_tab->h(), m_engine);
    m_project_tab->end();
}

} // namespace disgrace_ns
