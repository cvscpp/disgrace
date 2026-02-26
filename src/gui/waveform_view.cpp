#include "waveform_view.h"
#include <FL/fl_draw.H>

namespace disgrace_ns
{

    disgrace_ns::WaveformView::WaveformView(int x, int y,
                               int w, int h)
    : Fl_Widget(x,y,w,h)
    {
    }

    void disgrace_ns::WaveformView::set_sample(
        ::std::shared_ptr<disgrace_ns::SampleData> s)
    {
        m_sample = s;
        redraw();
    }

    void disgrace_ns::WaveformView::draw()
    {
        fl_color(20,20,20);
        fl_rectf(x(), y(), w(), h());

        if (!m_sample)
            return;

        fl_color(0,200,120);

        size_t size = m_sample->left.size();
        if (size == 0) return;

        for (int px = 0; px < w(); ++px)
        {
            size_t idx =
            (px * size) / w();

            float v = m_sample->left[idx];

            int yy = y() + h()/2;
            int amp = int(v * (h()/2));

            fl_line(x()+px,
                    yy - amp,
                    x()+px,
                    yy + amp);
        }
    }
    // load_btn->callback(
    //     [](Fl_Widget*, void* data)
    //     {
    //         auto* engine =
    //         static_cast<disgrace_ns::Engine*>(data);

    //         Fl_File_Chooser chooser(
    //             ".", "*.wav",
    //             Fl_File_Chooser::SINGLE,
    //             "Load Sample");

    //         chooser.show();
    //         while (chooser.shown())
    //             Fl::wait();

    //         if (chooser.value())
    //         {
    //             engine->load_sample_into_current(
    //                 chooser.value());
    //         }
    //     },
    //     // &engine);

} // namespace disgrace_ns
