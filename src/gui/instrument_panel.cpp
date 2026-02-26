#include "instrument_panel.h"
#include "detached_window.h"
#include "../core/engine.h"

namespace disgrace_ns {

InstrumentPanel::InstrumentPanel(int x, int y, int w, int h, Engine& engine)
    : Fl_Group(x, y, w, h), m_engine(engine) {
    
    begin();

    m_detach_btn = new Fl_Button(w - 30, 0, 30, 20, "[]"); // Relative to InstrumentPanel
    m_detach_btn->callback(cb_detach, this);

    m_inst_list = new Fl_Browser(0, 0, w, h - 30); // Relative to InstrumentPanel
    m_load_btn = new Fl_Button(0, h - 30, w, 30, "Load Sample"); // Relative to InstrumentPanel

    m_inst_list->callback(cb_inst_list, this);
    
    resizable(m_inst_list);

    end();
}

void InstrumentPanel::cb_inst_list(Fl_Widget* w, void* data) {
    InstrumentPanel* self = static_cast<InstrumentPanel*>(data);
    int idx = self->m_inst_list->value();
    if (idx > 0)
      self->m_engine.set_current_instrument(idx - 1);
}

void InstrumentPanel::cb_detach(Fl_Widget*, void* data) {
    InstrumentPanel* self = static_cast<InstrumentPanel*>(data);
    Fl_Group* parent = self->parent();
    if (self->m_detached_window) {
        self->m_detached_window->show();
    } else {
        self->m_detached_window = new DetachedWindow(200, 330, "Instruments", self, parent);
        self->m_detached_window->show();
    }
    self->hide(); // Hide the panel in its original location
}

} // namespace disgrace_ns
