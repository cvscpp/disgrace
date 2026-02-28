#include "mixer_panel.h"
#include "detached_window.h"
#include "../core/engine.h"
#include <FL/Fl_Scroll.H>

namespace disgrace_ns {

MixerPanel::MixerPanel(int x, int y, int w, int h, Engine& engine)
    : Fl_Group(x, y, w, h), m_engine(engine) {
    
    begin();

    m_detach_btn = new Fl_Button(w - 30, 0, 30, 20, "[]");
    m_detach_btn->callback(cb_detach, this);

    // Use a scroll area for tracks
    Fl_Scroll* scroll = new Fl_Scroll(0, 30, w, h - 70);
    scroll->type(Fl_Scroll::HORIZONTAL);

    // This group will hold the tracks
    m_track_group = new Fl_Group(0, 30, w, h - 70);
    m_track_group->end();
    scroll->add(m_track_group);
    scroll->end();

    m_master_gain = new Fl_Value_Slider(w - 150 - 5, h - 30, 150, 25, "Master");
    m_master_gain->type(FL_HOR_NICE_SLIDER);
    m_master_gain->range(0.0, 2.0);
    m_master_gain->value(1.0);
    m_master_gain->callback(cb_master_gain, this);

    Fl_Box* mixer_spacer = new Fl_Box(0, 0, w, h);
    mixer_spacer->hide();
    resizable(mixer_spacer);

    end();

    update_mixer_ui();
}

void MixerPanel::update_mixer_ui() {
    m_track_group->clear();
    m_track_group->begin();

    size_t num_tracks = m_engine.track_count();
    
    for (size_t i = 0; i < num_tracks; ++i)
    {
      int x_offset = (int)i * 100;
      new Fl_Box(20 + x_offset, 0, 80, 200, strdup(("Track " + ::std::to_string(i+1)).c_str()));
      
      Fl_Slider* vol = new Fl_Slider(20 + x_offset, 0, 20, 200);
      vol->type(FL_VERTICAL);
      vol->bounds(0, 1);
      vol->value(m_engine.track(i).volume());
      vol->callback(cb_track_volume, new ::std::pair<MixerPanel*,int>(this, (int)i));

      Fl_Check_Button* mute = new Fl_Check_Button(20 + x_offset, 210, 60, 20, "M");
      mute->value(m_engine.track(i).muted());
      mute->callback(cb_track_mute, new ::std::pair<MixerPanel*,int>(this, (int)i));
    }

    m_track_group->end();
    m_track_group->size((int)(num_tracks * 100 + 40), m_track_group->h());
    m_track_group->redraw();
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
}

void MixerPanel::cb_track_mute(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    MixerPanel* self = pair->first;
    bool m = static_cast<Fl_Check_Button*>(w)->value();
    self->m_engine.track(pair->second).set_mute(m);
}

void MixerPanel::cb_detach(Fl_Widget*, void* data) {
    MixerPanel* self = static_cast<MixerPanel*>(data);
    Fl_Group* parent_grp = self->parent();
    if (self->m_detached_window) {
        self->m_detached_window->show();
    } else {
        self->m_detached_window = new DetachedWindow(850, 300, "Mixer", self, parent_grp);
        self->m_detached_window->show();
    }
    self->hide();
}

} // namespace disgrace_ns
