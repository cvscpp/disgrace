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

        if (!m_sample || m_sample->left.empty()) {
            fl_color(100, 100, 100);
            fl_line(x(), y() + h()/2, x() + w(), y() + h()/2);
            return;
        }

        fl_color(0,200,120);

        size_t size = m_sample->left.size();
        bool stereo = !m_sample->right.empty() && m_sample->right.size() == size;

        if (!stereo) {
            // Mono drawing
            for (int px = 0; px < w(); ++px)
            {
                size_t idx = (px * size) / w();
                float v = m_sample->left[idx];
                int yy = y() + h()/2;
                int amp = int(v * (h()/2 - 2));
                fl_line(x()+px, yy - amp, x()+px, yy + amp);
            }
        } else {
            // Stereo drawing (split height)
            int half_h = h() / 2;
            
            // Left channel (Top half)
            fl_color(0, 200, 120);
            for (int px = 0; px < w(); ++px)
            {
                size_t idx = (px * size) / w();
                float v = m_sample->left[idx];
                int yy = y() + half_h / 2;
                int amp = int(v * (half_h / 2 - 2));
                fl_line(x()+px, yy - amp, x()+px, yy + amp);
            }

            // Right channel (Bottom half)
            fl_color(0, 180, 200);
            for (int px = 0; px < w(); ++px)
            {
                size_t idx = (px * size) / w();
                float v = m_sample->right[idx];
                int yy = y() + half_h + half_h / 2;
                int amp = int(v * (half_h / 2 - 2));
                fl_line(x()+px, yy - amp, x()+px, yy + amp);
            }
            
            // Separator line
            fl_color(60, 60, 60);
            fl_line(x(), y() + half_h, x() + w(), y() + half_h);
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
