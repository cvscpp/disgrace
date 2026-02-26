#include "main_window.h"
#include "mixer_panel.h"

namespace disgrace_ns {

void MainWindow::init_mixer_tab(int w, int h) {
    m_mixer_tab = new Fl_Group(0, 65, w, h - 100, "Mixer");
    m_mixer_tab->begin();
    m_mixer_panel = new MixerPanel(0, 0, m_mixer_tab->w(), m_mixer_tab->h(), m_engine);
    m_mixer_tab->resizable(m_mixer_panel);
    m_mixer_tab->end();
}

} // namespace disgrace_ns
