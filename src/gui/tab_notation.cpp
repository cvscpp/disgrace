#include "main_window.h"
#include "notation_panel.h"

namespace disgrace_ns {

void MainWindow::init_notation_tab(int w, int h) {
    m_notation_tab = new Fl_Group(0, 65, w, h - 100, "Notation");
    m_notation_panel = new NotationPanel(m_notation_tab->x(), m_notation_tab->y(), m_notation_tab->w(), m_notation_tab->h(), m_engine);
    m_notation_tab->end();
}

} // namespace disgrace_ns
