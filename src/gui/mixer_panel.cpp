#include "mixer_panel.h"
#include "detached_window.h"
#include "../core/engine.h"

namespace disgrace_ns {

MixerPanel::MixerPanel(int x, int y, int w, int h, Engine& engine)
    : Fl_Group(x, y, w, h), m_engine(engine) {
    
    begin();

    m_detach_btn = new Fl_Button(w - 30, 0, 30, 20, "[]");
    m_detach_btn->callback(cb_detach, this);

    for (int i = 0; i < 8; ++i)
    {
      new Fl_Box(20 + i*100, 30, 80, 200, ("Track " + ::std::to_string(i+1)).c_str());
      int x_track_vol = 20 + i * 100;

      Fl_Slider* vol = new Fl_Slider(x_track_vol, 30, 20, 200);
      vol->type(FL_VERTICAL);
      vol->bounds(0, 1);
      vol->value(1);
      vol->callback(cb_track_volume, new ::std::pair<MixerPanel*,int>(this, i));

      Fl_Check_Button* mute = new Fl_Check_Button(x_track_vol, 240, 60, 20, "M");
      mute->callback(cb_track_mute, new ::std::pair<MixerPanel*,int>(this, i));
    }
    m_master_gain = new Fl_Value_Slider(w - 150 - 5, h - 30, 150, 25, "Master");

    m_master_gain->type(FL_HOR_NICE_SLIDER);
    m_master_gain->range(0.0, 2.0);
    m_master_gain->value(1.0);
    m_master_gain->callback(cb_master_gain, this);

    Fl_Box* mixer_spacer = new Fl_Box(0, 0, w, h);
    mixer_spacer->hide();
    resizable(mixer_spacer);

    end();
}

void MixerPanel::cb_master_gain(Fl_Widget* w, void* data) {
    MixerPanel* self = static_cast<MixerPanel*>(data);
    float g = static_cast<Fl_Value_Slider*>(w)->value();
    self->m_engine.set_master_gain(g);
}

void MixerPanel::cb_track_volume(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    MixerPanel* self = pair->first;
    float v = static_cast<Fl_Slider*>(w)->value();
    self->m_engine.track(pair->second).set_volume(v);
    delete pair;
}

void MixerPanel::cb_track_mute(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    MixerPanel* self = pair->first;
    bool m = static_cast<Fl_Check_Button*>(w)->value();
    self->m_engine.track(pair->second).set_mute(m);
    delete pair;
}

void MixerPanel::cb_detach(Fl_Widget*, void* data) {
    MixerPanel* self = static_cast<MixerPanel*>(data);
    Fl_Group* parent = self->parent();
    if (self->m_detached_window) {
        self->m_detached_window->show();
    } else {
        self->m_detached_window = new DetachedWindow(850, 300, "Mixer", self, parent);
        self->m_detached_window->show();
    }
    self->hide(); // Hide the panel in its original location
}

} // namespace disgrace_ns
