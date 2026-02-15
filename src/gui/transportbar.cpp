#include "transportbar.h"
#include "../core/engine.h"

TransportBar::TransportBar(
    int X, int Y, int W, int H,
    Engine& engine)
: Fl_Group(X, Y, W, H),
m_engine(engine)
{
    int x = X + 10;

    m_play = new Fl_Button(x, Y+5, 60, 25, "Play");
    m_play->callback(cb_play, this);
    x += 70;

    m_stop = new Fl_Button(x, Y+5, 60, 25, "Stop");
    m_stop->callback(cb_stop, this);
    x += 70;

    m_record = new Fl_Light_Button(x, Y+5, 80, 25, "Record");
    m_record->callback(cb_record, this);
    x += 90;

    m_metronome = new Fl_Light_Button(x, Y+5, 100, 25, "Metronome");
    m_metronome->value(1);
    m_metronome->callback(cb_metronome, this);
    x += 110;

    m_tempo = new Fl_Value_Input(x, Y+5, 80, 25, "BPM");
    m_tempo->range(30, 300);
    m_tempo->step(1);
    m_tempo->value(m_engine.tempo());
    m_tempo->callback(cb_tempo, this);

    x += 110;


    m_status = new Fl_Box(x, Y+5, 200, 25, "Stopped");

    end();
}

void TransportBar::cb_tempo(Fl_Widget* w, void* data)
{
    auto* self = static_cast<TransportBar*>(data);

    double bpm =
    static_cast<Fl_Value_Input*>(w)->value();

    self->m_engine.set_tempo(bpm);
}


void TransportBar::cb_play(Fl_Widget*, void* data)
{
    auto* self = static_cast<TransportBar*>(data);
    self->m_engine.play();
}

void TransportBar::cb_stop(Fl_Widget*, void* data)
{
    auto* self = static_cast<TransportBar*>(data);
    self->m_engine.stop();
}

void TransportBar::cb_record(Fl_Widget*, void* data)
{
    auto* self = static_cast<TransportBar*>(data);

    bool rec = self->m_record->value();
    self->m_engine.enable_record(rec);

    if (rec)
        self->m_engine.record();
}

void TransportBar::cb_metronome(Fl_Widget*, void* data)
{
    auto* self = static_cast<TransportBar*>(data);

    self->m_engine.set_metronome_enabled(
        self->m_metronome->value());
}

void Timing::set_tempo(double bpm)
{
    m_tempo = bpm;

    double beats_per_second =
    bpm / 60.0;

    double rows_per_second =
    beats_per_second * 4.0;

    m_samples_per_row =
    static_cast<size_t>(
        m_sample_rate / rows_per_second);
}


void TransportBar::update()
{
    double engine_bpm =
    m_engine.tempo();

    if (m_tempo->value() != engine_bpm)
        m_tempo->value(engine_bpm);

    auto state = m_engine.transport_state();

    switch (state)
    {
        case TransportState::Stopped:
            m_status->label("Stopped");
            break;

        case TransportState::Playing:
            m_status->label("Playing");
            break;

        case TransportState::Recording:
            m_status->label("Recording");
            break;
    }

    redraw();
}
