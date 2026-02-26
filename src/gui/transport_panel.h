#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Value_Input.H>

namespace disgrace_ns
{

    class Engine;

    class TransportPanel : public Fl_Group
    {
    public:
        TransportPanel(int x, int y, int w, int h, disgrace_ns::Engine& engine);

    private:
        static void cb_play(Fl_Widget*, void*);
        static void cb_stop(Fl_Widget*, void*);
        static void cb_tempo(Fl_Widget*, void*);

        disgrace_ns::Engine& m_engine;

        Fl_Button*      m_play;
        Fl_Button*      m_stop;
        Fl_Value_Input* m_tempo;
    };

} // namespace disgrace_ns
