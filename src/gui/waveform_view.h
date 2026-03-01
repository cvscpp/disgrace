#pragma once
#include <FL/Fl_Widget.H>
#include <memory>
#include "../audio/sample_data.h"
#include "FL/Fl_File_Chooser.H"

namespace disgrace_ns
{

    class WaveformView : public Fl_Widget
    {
    public:
        WaveformView(int x, int y, int w, int h);

        void set_sample(::std::shared_ptr<disgrace_ns::SampleData> s);
        void draw() override;

    private:
        ::std::shared_ptr<disgrace_ns::SampleData> m_sample;
    };

} // namespace disgrace_ns
