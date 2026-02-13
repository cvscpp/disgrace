#include "main_window.h"
#include "transport_panel.h"
#include "tracker_view.h"
#include "../engine/engine.h"

#include <FL/fl_draw.H>

namespace dg
{

  MainWindow::MainWindow(int w, int h,
                         const char* title,
                         Engine& engine)
  : Fl_Double_Window(w, h, title),
  m_engine(engine)
  {
    begin();

    m_menu = new Fl_Menu_Bar(0, 0, w, 25);
    m_menu->add("&File/&Quit", FL_CTRL + 'q',
                [](Fl_Widget*, void*)
                {
                  std::exit(0);
                });

    m_transport = new TransportPanel(0, 25, w, 40, m_engine);

    Fl_Box* pattern_list =
    new Fl_Box(10, 100, 120,
               h - 150,
               "Pattern 00\nPattern 01\nPattern 02");

    m_tracker =
    new TrackerView(140, 100,
                    w - 150,
                    h - 150,
                    engine.pattern(),
                    engine);


    m_tracker = new TrackerView(0, 65, w, h - 100);

    m_status = new Fl_Box(0, h - 35, w, 35);
    m_status->box(FL_FLAT_BOX);
    m_status->label("disgrace 0.1 – ready");
    m_status->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    m_tabs = new Fl_Tabs(0, 65, w, h - 100);

    m_tracker_tab = new Fl_Group(0, 90, w, h - 125, "Tracker");
    Fl_Scroll* scroll =
    new Fl_Scroll(140, 100,
                  w - 150,
                  h - 150);

    m_tracker =
    new TrackerView(140, 100,
                    w - 170,
                    64 * 18,
                    engine.pattern(),
                    engine);

    scroll->end();
    Fl_Check_Button* loop_btn =
    new Fl_Check_Button(200, 30,
                        60, 25,
                        "Loop");

    loop_btn->value(1);

    loop_btn->callback(
      [](Fl_Widget* w, void* data)
      {
        auto* engine =
        static_cast<Engine*>(data);

        engine->set_loop(
          static_cast<Fl_Check_Button*>(w)
          ->value());
      },
      &engine);
    if (m_engine.transport() ==
      TransportState::Stopped)
    {
      m_engine.set_play_position(
        m_engine.active_pattern(),
                                 m_cursor_row);
    }
    m_engine.toggle_play();

    m_tracker_tab->end();

    m_mixer_tab = new Fl_Group(0, 90, w, h - 125, "Mixer");
    for (int i = 0; i < 8; ++i)
    {
      new Fl_Box(20 + i*100, 120,
                 80, 200,
                 ("Track " + std::to_string(i+1)).c_str());
      int x = 20 + i * 100;

      Fl_Slider* vol =
      new Fl_Slider(x, 120, 20, 200);
      vol->type(FL_VERTICAL);
      vol->bounds(0, 1);
      vol->value(1);

      vol->callback(
        [](Fl_Widget* w, void* data)
        {
          auto* pair =
          static_cast<std::pair<Engine*,int>*>(data);

          float v =
          static_cast<Fl_Slider*>(w)->value();

          pair->first->track(pair->second)
          .set_volume(v);
        },
        new std::pair<Engine*,int>(&engine, i));

      Fl_Check_Button* mute =
      new Fl_Check_Button(x, 330, 60, 20, "M");
      mute->callback(
        [](Fl_Widget* w, void* data)
        {
          auto* pair =
          static_cast<std::pair<Engine*,int>*>(data);

          bool m =
          static_cast<Fl_Check_Button*>(w)->value();

          pair->first->track(pair->second)
          .set_mute(m);
        },
        new std::pair<Engine*,int>(&engine, i));
    }
    m_mixer_tab->end();

    m_tabs->end();
    Fl_Browser* pattern_list =
    new Fl_Browser(10, 100, 120, h - 150);

    for (int i = 0; i < 3; ++i)
    {
      pattern_list->add(
        ("Pattern " + std::to_string(i)).c_str());
    }

    pattern_list->callback(
      [](Fl_Widget* w, void* data)
      {
        auto* browser =
        static_cast<Fl_Browser*>(w);
        auto* engine =
        static_cast<Engine*>(data);

        int selected = browser->value();
        if (selected > 0)
          engine->set_active_pattern(
            selected - 1);
      },
      &engine);
    m_instrument_tab =
    new Fl_Group(0, 90, w, h - 125, "Instruments");
    Fl_Browser* inst_list =
    new Fl_Browser(10, 100, 200, 300);

    Fl_Button* load_btn =
    new Fl_Button(10, 410, 200, 30, "Load Sample");

    inst_list->callback(
      [](Fl_Widget* w, void* data)
      {
        auto* browser =
        static_cast<Fl_Browser*>(w);
        auto* engine =
        static_cast<Engine*>(data);

        int idx = browser->value();
        if (idx > 0)
          engine->set_current_instrument(idx - 1);
      },
      &engine);

    end();

    resizable(m_tracker);
  }
  Fl::add_timeout(0.03,
                  [](void* userdata)
                  {
                    auto* self =
                    static_cast<MainWindow*>(userdata);

                    self->redraw();
                    Fl::repeat_timeout(0.03,
                                       this,
                                       userdata);
                  },
                  this);


}
