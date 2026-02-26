#include "main_window.h"
#include "transport_panel.h"
#include "tracker_panel.h"
#include "../core/engine.h"

#include <FL/fl_draw.H>
#include <FL/Fl_Native_File_Chooser.H> // ADD THIS LINE for fl_file_chooser

namespace disgrace_ns
{

  disgrace_ns::MainWindow::MainWindow(int w, int h,
                         const char* title,
                         disgrace_ns::Engine& engine)
  : Fl_Double_Window(w, h, title),
  m_engine(engine),
  m_transport(nullptr),
  m_status(nullptr),
  m_tabs(nullptr),
  m_project_tab(nullptr),
  m_tracker_tab(nullptr),
  m_mixer_tab(nullptr),
  m_instrument_tab(nullptr),
  m_settings_tab(nullptr),
  m_loop_btn(nullptr),
  m_tracker_panel(nullptr),
  m_instrument_panel(nullptr),
  m_mixer_panel(nullptr),
  m_settings_panel(nullptr)
  {
    begin();

    m_transport = new disgrace_ns::TransportPanel(0, 0, w, 40, m_engine);
    m_loop_btn = new Fl_Check_Button(w - 70, 10, 60, 25, "Loop");
    m_loop_btn->value(1);
    m_loop_btn->callback(
      [](Fl_Widget* w, void* data)
      {
        auto* engine =
        static_cast<disgrace_ns::Engine*>(data);

        engine->set_loop(
          static_cast<Fl_Check_Button*>(w)
          ->value());
      },
      &m_engine);

    m_tabs = new Fl_Tabs(0, 40, w, h - 75);
    
    init_project_tab(w, h);
    init_tracker_tab(w, h);
    init_instrument_tab(w, h);
    init_mixer_tab(w, h);
    init_settings_tab(w, h);

    m_tabs->end();

    resizable(m_tabs);
    end();
  }

  disgrace_ns::MainWindow::~MainWindow() {}

  disgrace_ns::Track& disgrace_ns::MainWindow::track(size_t index)
  {
      return m_engine.track(index);
  }

void disgrace_ns::MainWindow::timer_cb(void* data)
  {
    auto* self = static_cast<disgrace_ns::MainWindow*>(data);

    // This check is no longer valid as m_tracker is now in TrackerPanel
    // if (self->m_tracker == nullptr) {
    //     fprintf(stderr, "DEBUG: m_tracker is nullptr in timer_cb! Skipping set_current_row.\n");
    //     Fl::repeat_timeout(0.03, timer_cb, data);
    //     return;
    // }
    
    Fl::repeat_timeout(0.03, timer_cb, data);
  }

  int disgrace_ns::MainWindow::handle(int event){
    if (event == FL_KEYDOWN)
    {
      switch (Fl::event_key())
      {
        case ' ':
          m_engine.play();
          return 1;

        case 'r':
          m_engine.record();
          return 1;

        case 'm':
          m_engine.toggle_metronome();
          return 1;
      }
    }
    return Fl_Double_Window::handle(event); // Call base class handle for unhandled events
  }

} // namespace disgrace_ns
