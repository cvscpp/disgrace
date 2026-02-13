#pragma once
#include <FL/Fl_Widget.H>
#include <vector>
#include "../pattern/pattern.h"

namespace dg
{

    class Engine;

    class TrackerView : public Fl_Widget
    {
    public:
        TrackerView(int x, int y, int w, int h,
                    Pattern& pattern,
                    Engine& engine);

        void draw() override;
        int handle(int event) override;

    private:
        Pattern& m_pattern;
        Engine&  m_engine;

        int m_cursor_row = 0;
        int m_cursor_track = 0;

        int m_rows  = 64;
        int m_tracks = 8;

        bool m_selecting = false;

        int m_sel_row_start = -1;
        int m_sel_track_start = -1;

        int m_sel_row_end = -1;
        int m_sel_track_end = -1;


        void insert_note(uint8_t note);
    };

}
