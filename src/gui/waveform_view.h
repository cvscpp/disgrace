#pragma once
#include <FL/Fl_Widget.H>
#include <memory>
#include "../audio/sample_data.h"
#include "FL/Fl_File_Chooser.H"

namespace disgrace_ns
{

    enum class ChannelMode { Both, Left, Right };

    class WaveformView : public Fl_Widget
    {
    public:
        WaveformView(int x, int y, int w, int h);

        void set_sample(::std::shared_ptr<disgrace_ns::SampleData> s);
        void set_color(unsigned int c) { m_color = c; redraw(); }
        
        void draw() override;
        int handle(int event) override;

        void zoom_in();
        void zoom_out();
        void view_all();
        void view_selection();

        void set_channel_mode(ChannelMode mode) { m_mode = mode; redraw(); }
        
        size_t selection_start() const { return m_sel_start; }
        size_t selection_end() const { return m_sel_end; }

    private:
        ::std::shared_ptr<disgrace_ns::SampleData> m_sample;
        unsigned int m_color = 0x40FF4000;
        
        size_t m_sel_start = 0;
        size_t m_sel_end = 0;
        
        double m_zoom = 1.0;
        size_t m_offset = 0;
        
        ChannelMode m_mode = ChannelMode::Both;

        void get_view_range(size_t& start, size_t& end);
    };

} // namespace disgrace_ns
