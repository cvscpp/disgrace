#include "main_window.h"
#include "settings_panel.h"

namespace disgrace_ns {

void MainWindow::init_settings_tab(int w, int h) {
    m_settings_tab = new Fl_Group(0, 65, w, h - 100, "Settings");
    m_settings_panel = new SettingsPanel(m_settings_tab->x(), m_settings_tab->y(), m_settings_tab->w(), m_settings_tab->h(), m_engine);
    m_settings_tab->end();
}

} // namespace disgrace_ns
