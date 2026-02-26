#pragma once

#include <FL/Fl_Group.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>

namespace disgrace_ns {

class Engine;
class DetachedWindow;

class MixerPanel : public Fl_Group {
public:
    MixerPanel(int x, int y, int w, int h, Engine& engine);

private:
    Engine& m_engine;
    Fl_Value_Slider* m_master_gain;
    Fl_Button* m_detach_btn;
    DetachedWindow* m_detached_window = nullptr;

    static void cb_master_gain(Fl_Widget*, void*);
    static void cb_track_volume(Fl_Widget*, void*);
    static void cb_track_mute(Fl_Widget*, void*);
    static void cb_detach(Fl_Widget*, void*);
};

} // namespace disgrace_ns
