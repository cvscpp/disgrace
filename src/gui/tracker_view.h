#pragma once
#include <FL/Fl_Widget.H>
#include <vector>
#include "../sequencer/pattern.h"

namespace disgrace_ns
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
        void draw_track_header(int track_index, int x, int y, int w, int h);
        void set_current_row(int row);
        void recalculate_size();
        void ensure_cursor_visible();

    private:
        void delete_current_field();
        Pattern& m_pattern;
        Engine&  m_engine;

        struct TrackUI {
            int x, w;
            int btn_plus_x, btn_minus_x;
        };
        std::vector<TrackUI> m_track_ui;

        int m_cursor_row = 0;
        int m_cursor_track = 0;
        int m_cursor_col = 0;
        int m_cursor_field = 0; // 0: note, 1: sample, 2: vol, 3: fx1, 4: fx2

        bool m_sel_active = false;
        int m_sel_start_track = -1;
        int m_sel_start_row = -1;
        int m_sel_end_track = -1;
        int m_sel_end_row = -1;

        int m_rows  = 64;
        int m_tracks = 8;

        bool m_selecting = false;

        int m_sel_row_start = -1;
        int m_sel_track_start = -1;

        int m_sel_row_end = -1;
        int m_sel_track_end = -1;


        void insert_note(uint8_t note);
    };

} // namespace disgrace_ns
