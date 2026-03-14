#include "main_window.h"

namespace disgrace_ns {

void MainWindow::init_notation_tab(int w, int h) {
    m_notation_tab = new Fl_Group(0, 65, w, h - 100, "Notation");
    m_notation_tab->begin();
    m_notation_tab->end();
}

} // namespace disgrace_ns
