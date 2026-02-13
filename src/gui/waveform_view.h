#pragma once
#include <FL/Fl_Widget.H>
#include "../audio/sample.h"
#include "FL/Fl_File_Chooser.H>"

namespace dg
{

    class WaveformView : public Fl_Widget
    {
    public:
        WaveformView(int x, int y, int w, int h);

        void set_sample(std::shared_ptr<Sample> s);
        void draw() override;

    private:
        std::shared_ptr<Sample> m_sample;
    };

}
