#include "tracker_view.h"
#include "../core/engine.h"
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <algorithm>

namespace disgrace_ns {

TrackerView::TrackerView(int x, int y, int w, int h, Pattern& pattern, Engine& engine)
    : Fl_Widget(x, y, w, h), m_pattern(pattern), m_engine(engine) {
}

void TrackerView::draw() {
    fl_push_clip(x(), y(), w(), h());
    fl_color(30, 30, 30);
    fl_rectf(x(), y(), w(), h());

    int row_h = 18;
    int char_w = 8;
    int margin = 5;
    
    size_t num_tracks = m_engine.track_count();
    size_t num_rows = m_pattern.row_count();

    m_track_ui.clear();

    // Draw row numbers
    fl_color(100, 100, 100);
    fl_font(FL_COURIER, 12);
    for (size_t r = 0; r < num_rows; ++r) {
        int ry = y() + 20 + (int)r * row_h;
        if (ry < y() + 20 - row_h) continue;
        if (ry > y() + h()) break;
        
        char buf[8];
        snprintf(buf, 8, "%03zu", r);
        fl_draw(buf, x() + 2, ry + 14);
    }

    int cur_x = x() + 40;

    for (size_t t = 0; t < num_tracks; ++t) {
        auto& track_obj = m_engine.track(t);
        bool is_sampler = (track_obj.instrument() && track_obj.instrument()->type() == InstrumentType::Sampler);
        size_t num_cols = m_pattern.column_count(t);
        
        int track_w = (int)(num_cols * 10 * char_w + 2 * 4 * char_w + 40); // Added padding for buttons
        
        TrackUI ui;
        ui.x = cur_x;
        ui.w = track_w;
        ui.btn_plus_x = cur_x + track_w - 20;
        ui.btn_minus_x = cur_x + track_w - 40;
        m_track_ui.push_back(ui);

        // Draw track header
        fl_color(60, 60, 60);
        fl_rectf(cur_x, y(), track_w, 20);
        fl_color(200, 200, 200);
        fl_draw(track_obj.name().c_str(), cur_x + 5, y() + 15);

        // Draw + / - buttons
        fl_color(80, 80, 80);
        fl_rectf(ui.btn_minus_x, y() + 2, 18, 16);
        fl_rectf(ui.btn_plus_x, y() + 2, 18, 16);
        fl_color(255, 255, 255);
        fl_draw("-", ui.btn_minus_x + 5, y() + 14);
        fl_draw("+", ui.btn_plus_x + 5, y() + 14);
        
        // Draw grid and data
        for (size_t r = 0; r < num_rows; ++r) {
            int ry = y() + 20 + (int)r * row_h;
            if (ry < y() + 20 - row_h) continue;
            if (ry > y() + h()) break;

            if ((int)r == m_cursor_row) {
                fl_color(60, 60, 80);
                fl_rectf(cur_x, ry, track_w, row_h);
            }

            int col_x = cur_x + 2;
            for (size_t c = 0; c < num_cols; ++c) {
                const auto& ev = m_pattern.event(t, r, c);
                
                // Note
                if (ev.note == 255) {
                    fl_color(80, 80, 80);
                    fl_draw("---", col_x, ry + 14);
                } else {
                    fl_color(255, 255, 255);
                    const char* notes[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};
                    char buf[8];
                    snprintf(buf, 8, "%s%d", notes[ev.note % 12], ev.note / 12);
                    fl_draw(buf, col_x, ry + 14);
                }
                col_x += 4 * char_w;

                // Sample (only for sampler instruments)
                if (is_sampler) {
                    fl_color(0, 200, 200);
                    if (ev.sample_idx == 0) fl_draw("..", col_x, ry + 14);
                    else {
                        char buf[4]; snprintf(buf, 4, "%02X", ev.sample_idx);
                        fl_draw(buf, col_x, ry + 14);
                    }
                } else {
                    fl_color(40, 40, 40);
                    fl_draw("..", col_x, ry + 14);
                }
                col_x += 3 * char_w;

                // Volume
                fl_color(0, 255, 0);
                if (ev.volume == 255) fl_draw("..", col_x, ry + 14);
                else {
                    char buf[4]; snprintf(buf, 4, "%02X", ev.volume);
                    fl_draw(buf, col_x, ry + 14);
                }
                col_x += 3 * char_w;
            }

            const auto& row_ev = m_pattern.event(t, r, 0); 
            fl_color(255, 255, 0);
            char fx1[8]; snprintf(fx1, 8, "%02X%02X", row_ev.effect1, row_ev.param1);
            fl_draw(fx1, col_x, ry + 14);
            col_x += 5 * char_w;
            
            char fx2[8]; snprintf(fx2, 8, "%02X%02X", row_ev.effect2, row_ev.param2);
            fl_draw(fx2, col_x, ry + 14);
        }

        fl_color(50, 50, 50);
        fl_line(cur_x + track_w, y(), cur_x + track_w, y() + h());
        cur_x += track_w + 10;
    }

    fl_pop_clip();
}

int TrackerView::handle(int event) {
    switch (event) {
        case FL_PUSH: {
            take_focus();
            int mx = Fl::event_x();
            int my = Fl::event_y();
            
            // Check if we clicked on a header button
            if (my >= y() && my < y() + 20) {
                for (size_t t = 0; t < m_track_ui.size(); ++t) {
                    const auto& ui = m_track_ui[t];
                    if (mx >= ui.btn_plus_x && mx < ui.btn_plus_x + 18) {
                        m_pattern.set_column_count(t, m_pattern.column_count(t) + 1);
                        redraw();
                        return 1;
                    }
                    if (mx >= ui.btn_minus_x && mx < ui.btn_minus_x + 18) {
                        size_t current = m_pattern.column_count(t);
                        if (current > 1) {
                            m_pattern.set_column_count(t, current - 1);
                            redraw();
                        }
                        return 1;
                    }
                }
            }
            return 1;
        }
        case FL_KEYDOWN:
            return 1;
    }
    return Fl_Widget::handle(event);
}

void TrackerView::set_current_row(int row) {
    m_cursor_row = row;
    redraw();
}

void TrackerView::insert_note(uint8_t note) {
    if (m_cursor_track < (int)m_engine.track_count()) {
        m_pattern.event(m_cursor_track, m_cursor_row, 0).note = note;
        redraw();
    }
}

} // namespace disgrace_ns
