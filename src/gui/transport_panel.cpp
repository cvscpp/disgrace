#include "transport_panel.h"
#include "../engine/engine.h"

namespace dg
{

    TransportPanel::TransportPanel(int x, int y,
                                   int w, int h,
                                   Engine& engine)
    : Fl_Group(x, y, w, h),
    m_engine(engine)
    {
        begin();

        m_play = new Fl_Button(x + 10, y + 5, 60, 30, "Play");
        m_play->callback(cb_play, this);

        m_stop = new Fl_Button(x + 80, y + 5, 60, 30, "Stop");
        m_stop->callback(cb_stop, this);

        m_tempo = new Fl_Value_Input(x + 160, y + 5, 80, 30, "BPM");
        m_tempo->value(125);
        m_tempo->callback(cb_tempo, this);

        end();
    }

    void TransportPanel::cb_play(Fl_Widget*, void* userdata)
    {
        auto* self = static_cast<TransportPanel*>(userdata);
        self->m_engine.play();
    }

    void TransportPanel::cb_stop(Fl_Widget*, void* userdata)
    {
        auto* self = static_cast<TransportPanel*>(userdata);
        self->m_engine.stop();
    }

    void TransportPanel::cb_tempo(Fl_Widget*, void* userdata)
    {
        auto* self = static_cast<TransportPanel*>(userdata);
        self->m_engine.set_tempo(self->m_tempo->value());
    }

}
