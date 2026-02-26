#include "main_window.h"
#include "../core/engine.h"

namespace disgrace_ns {

void MainWindow::init_project_tab(int w, int h) {
    m_project_tab = new Fl_Group(0, 65, w, h - 100, "Project");
    m_project_tab->begin();
    Fl_Box* project_info = new Fl_Box(20, 80, 200, 25, "Project Settings");
    project_info->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    
    Fl_Button* save_btn = new Fl_Button(20, 110, 100, 25, "Save Project");
    save_btn->callback([](Fl_Widget*, void* data) {
        auto* engine = static_cast<disgrace_ns::Engine*>(data);
        engine->save_project("project.dg"); // Placeholder path
    }, &m_engine);

    m_project_tab->end();
}

} // namespace disgrace_ns
