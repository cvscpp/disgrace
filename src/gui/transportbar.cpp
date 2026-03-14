#include "transportbar.h"
#include "../core/engine.h"
#include <FL/Fl_Check_Button.H>
#include <cstring>
#include <cstdio>

namespace disgrace_ns
{

TransportBar::TransportBar(int X, int Y, int W, int H, Engine& engine)
: Fl_Group(X, Y, W, H), m_engine(engine)
{
    int x = X + 5;
    int btn_w = 60;
    int btn_h = 25;
    int btn_spacing = 4;

    m_play = new Fl_Button(x, Y+5, btn_w, btn_h, "Play");
    m_play->callback(cb_play, this);
    x += btn_w + btn_spacing;

    m_stop = new Fl_Button(x, Y+5, btn_w, btn_h, "Stop");
    m_stop->callback(cb_stop, this);
    x += btn_w + btn_spacing;

    m_record = new Fl_Light_Button(x, Y+5, btn_w, btn_h, "Edit");
    m_record->callback(cb_record, this);
    x += btn_w + btn_spacing;

    m_loop = new Fl_Light_Button(x, Y+5, btn_w, btn_h, "Loop");
    m_loop->value(1);
    m_loop->callback(cb_loop, this);
    x += btn_w + btn_spacing;

    m_metronome = new Fl_Light_Button(x, Y+5, btn_w + 10, btn_h, "Metro");
    m_metronome->value(1);
    m_metronome->callback(cb_metronome, this);
    x += btn_w + 10 + btn_spacing + 10;

    int counter_w = 35;
    int label_w = 35;

    m_tempo = new Fl_Simple_Counter(x + label_w, Y+5, counter_w, btn_h, "BPM");
    m_tempo->range(30, 300);
    m_tempo->step(1);
    m_tempo->value(m_engine.tempo());
    m_tempo->callback(cb_tempo, this);
    m_tempo->align(FL_ALIGN_LEFT);
    x += label_w + counter_w + btn_spacing + 5;

    m_lpb = new Fl_Simple_Counter(x + label_w, Y+5, counter_w, btn_h, "LPB");
    m_lpb->range(1, 128);
    m_lpb->step(1);
    m_lpb->value(m_engine.lpb());
    m_lpb->callback(cb_lpb, this);
    m_lpb->align(FL_ALIGN_LEFT);
    x += label_w + counter_w + btn_spacing + 5;

    Fl_Simple_Counter* octave_in = new Fl_Simple_Counter(x + label_w, Y+5, counter_w, btn_h, "Oct");
    octave_in->range(0, 9);
    octave_in->step(1);
    octave_in->value(m_engine.base_octave());
    octave_in->align(FL_ALIGN_LEFT);
    octave_in->callback([](Fl_Widget* w, void* d){
        ((Engine*)d)->set_base_octave((int)((Fl_Simple_Counter*)w)->value());
    }, &m_engine);
    x += label_w + counter_w + btn_spacing + 5;

    m_step = new Fl_Simple_Counter(x + label_w, Y+5, counter_w, btn_h, "Step");
    m_step->range(1, 64);
    m_step->step(1);
    m_step->value(1);
    m_step->callback(cb_step, this);
    m_step->align(FL_ALIGN_LEFT);
    x += label_w + counter_w + btn_spacing + 10;

    m_status = new Fl_Box(x, Y+5, 70, 25, "Stopped");
    m_status->labelfont(FL_HELVETICA_BOLD);
    x += 75;

    m_clock = new Fl_Box(x, Y+5, 100, 25, "00:00.000");
    m_clock->labelfont(FL_COURIER_BOLD);
    m_clock->labelsize(16);
    m_clock->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
    x += 105;

    // Allocate remaining space to meters
    int meter_w = W - x - 10;
    if (meter_w < 150) meter_w = 150; 

    m_meter_l = new VUMeter(x, Y+5, meter_w, 10, m_engine, nullptr, true);
    m_meter_r = new VUMeter(x, Y+18, meter_w, 10, m_engine, nullptr, true);

    memset(m_clock_str, 0, sizeof(m_clock_str));
    strcpy(m_clock_str, "00:00.000");
    m_clock->label(m_clock_str);

    end();
}

void TransportBar::cb_tempo(Fl_Widget* w, void* data)
{
    auto* self = static_cast<disgrace_ns::TransportBar*>(data);
    self->m_engine.set_tempo(static_cast<Fl_Simple_Counter*>(w)->value());
}

void TransportBar::cb_lpb(Fl_Widget* w, void* data)
{
    auto* self = static_cast<disgrace_ns::TransportBar*>(data);
    self->m_engine.set_lpb((uint32_t)static_cast<Fl_Simple_Counter*>(w)->value());
}

void TransportBar::cb_step(Fl_Widget* w, void* data)
{
    auto* self = static_cast<disgrace_ns::TransportBar*>(data);
    self->m_engine.set_step_size((uint32_t)static_cast<Fl_Simple_Counter*>(w)->value());
}

void TransportBar::cb_loop(Fl_Widget* w, void* data)
{
    auto* self = static_cast<disgrace_ns::TransportBar*>(data);
    self->m_engine.set_loop(static_cast<Fl_Light_Button*>(w)->value());
}

void TransportBar::cb_play(Fl_Widget*, void* data)
{
    auto* self = static_cast<disgrace_ns::TransportBar*>(data);
    self->m_engine.play();
}

void TransportBar::cb_stop(Fl_Widget*, void* data)
{
    auto* self = static_cast<disgrace_ns::TransportBar*>(data);
    self->m_engine.stop();
}

void TransportBar::cb_record(Fl_Widget*, void* data)
{
    auto* self = static_cast<disgrace_ns::TransportBar*>(data);
    bool rec = self->m_record->value();
    self->m_engine.enable_record(rec);
}

void TransportBar::cb_metronome(Fl_Widget*, void* data)
{
    auto* self = static_cast<disgrace_ns::TransportBar*>(data);
    self->m_engine.set_metronome_enabled(self->m_metronome->value());
}

void TransportBar::update()
{
    double engine_bpm = m_engine.tempo();
    if (m_tempo->value() != engine_bpm) m_tempo->value(engine_bpm);

    uint32_t engine_lpb = m_engine.lpb();
    if (m_lpb->value() != (double)engine_lpb) m_lpb->value((double)engine_lpb);

    if (m_record->value() != (int)m_engine.m_record_enabled) 
        m_record->value(m_engine.m_record_enabled);

    auto state = m_engine.transport_state();
    switch (state)
    {
        case disgrace_ns::TransportState::Stopped: m_status->label("Stopped"); break;
        case disgrace_ns::TransportState::Playing: m_status->label("Playing"); break;
    }

    double total_seconds = m_engine.get_current_time_seconds();
    int minutes = (int)(total_seconds / 60);
    double seconds = total_seconds - (minutes * 60);
    
    snprintf(m_clock_str, 32, "%02d:%06.3f", minutes, seconds);
    m_clock->label(m_clock_str);

    if (m_meter_l) m_meter_l->level(m_engine.master_meter_l());
    if (m_meter_r) m_meter_r->level(m_engine.master_meter_r());
    redraw();
}

void TransportBar::resize(int X, int Y, int W, int H) {
    Fl_Group::resize(X, Y, W, H);
    
    // Recalculate meter width to fill remaining space
    int x_meters = m_meter_l->x();
    int new_meter_w = (X + W) - x_meters - 10;
    if (new_meter_w < 100) new_meter_w = 100;
    
    m_meter_l->size(new_meter_w, m_meter_l->h());
    m_meter_r->size(new_meter_w, m_meter_r->h());
}

} // namespace disgrace_ns
