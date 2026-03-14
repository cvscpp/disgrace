#include "main_window.h"
#include "instrument_panel.h"

namespace disgrace_ns {

void MainWindow::init_instrument_tab(int w, int h) {
    m_instrument_tab = new Fl_Group(0, 65, w, h - 100, "Instruments");
    m_instrument_panel = new InstrumentPanel(m_instrument_tab->x(), m_instrument_tab->y(), m_instrument_tab->w(), m_instrument_tab->h(), m_engine);
    m_instrument_tab->end();
}

} // namespace disgrace_ns
