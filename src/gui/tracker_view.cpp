#include "tracker_view.h"
#include "../core/engine.h"
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <algorithm>

namespace disgrace_ns {

TrackerView::TrackerView(int x, int y, int w, int h, Pattern& pattern, Engine& engine)
    : Fl_Widget(x, y, w, h), m_pattern(pattern), m_engine(engine) {
}

void TrackerView::draw() {
    recalculate_size();
    fl_push_clip(x(), y(), w(), h());
    fl_color(30, 30, 30);
    fl_rectf(x(), y(), w(), h());

    if (Fl::focus() == this) {
        fl_color(255, 0, 0);
        fl_rect(x(), y(), w(), h());
    }

    int row_h = 18;
    int char_w = 8;
    
    size_t num_tracks = m_engine.track_count();
    size_t num_rows = m_pattern.row_count();

    // Clamp cursor
    if (m_cursor_track >= (int)num_tracks) m_cursor_track = (int)num_tracks - 1;
    if (m_cursor_row >= (int)num_rows) m_cursor_row = (int)num_rows - 1;
    if (m_cursor_track < 0) m_cursor_track = 0;
    if (m_cursor_row < 0) m_cursor_row = 0;

    m_track_ui.clear();

    if (num_tracks == 0) {
        fl_color(200, 200, 200);
        fl_font(FL_HELVETICA, 16);
        fl_draw("No tracks. Click '+ Track' in the transport bar to add one.", x() + 50, y() + 50);
        fl_pop_clip();
        return;
    }

    // Draw row numbers
    fl_color(100, 100, 100);
    fl_font(FL_COURIER, 12);
    for (size_t r = 0; r < num_rows; ++r) {
        int ry = y() + 20 + (int)r * row_h;
        if (ry < y() + 20 - row_h) continue;
        if (ry > y() + h()) break;
        char buf[8]; snprintf(buf, 8, "%03zu", r);
        fl_draw(buf, x() + 2, ry + 14);
    }

    int cur_x = x() + 40;
    for (size_t t = 0; t < num_tracks; ++t) {
        auto& track_obj = m_engine.track(t);
        bool is_sampler = (track_obj.instrument() && track_obj.instrument()->type() == InstrumentType::Sampler);
        size_t num_cols = m_pattern.column_count(t);
        int track_w = (int)(num_cols * 10 * char_w + 2 * 4 * char_w + 40);
        
        TrackUI ui; ui.x = cur_x; ui.w = track_w; ui.btn_plus_x = cur_x + track_w - 20; ui.btn_minus_x = cur_x + track_w - 40;
        m_track_ui.push_back(ui);

        fl_color(60, 60, 60); fl_rectf(cur_x, y(), track_w, 20);
        fl_color(200, 200, 200); fl_draw(track_obj.name().substr(0, 10).c_str(), cur_x + 5, y() + 15);
        fl_color(80, 80, 80); fl_rectf(ui.btn_minus_x, y() + 2, 18, 16); fl_rectf(ui.btn_plus_x, y() + 2, 18, 16);
        fl_color(255, 255, 255); fl_draw("-", ui.btn_minus_x + 5, y() + 14); fl_draw("+", ui.btn_plus_x + 5, y() + 14);
        
        for (size_t r = 0; r < num_rows; ++r) {
            int ry = y() + 20 + (int)r * row_h;
            if (ry < y() + 20 - row_h) continue;
            if (ry > y() + h()) break;

            // Selection highlight
            if (m_sel_active) {
                int s_r = std::min(m_sel_start_row, m_sel_end_row);
                int e_r = std::max(m_sel_start_row, m_sel_end_row);
                int s_t = std::min(m_sel_start_track, m_sel_end_track);
                int e_t = std::max(m_sel_start_track, m_sel_end_track);
                if ((int)r >= s_r && (int)r <= e_r && (int)t >= s_t && (int)t <= e_t) {
                    fl_color(80, 80, 40); fl_rectf(cur_x, ry, track_w, row_h);
                }
            }

            if ((int)r == m_cursor_row && (int)t == m_cursor_track) {
                fl_color(60, 60, 80); fl_rectf(cur_x, ry, track_w, row_h);
            }

            int col_x = cur_x + 2;
            for (size_t c = 0; c < num_cols; ++c) {
                const auto& ev = m_pattern.event(t, r, c);
                
                // Note
                if ((int)r == m_cursor_row && (int)t == m_cursor_track && (int)c == m_cursor_col && m_cursor_field == 0) {
                    fl_color(100, 100, 150); fl_rectf(col_x - 1, ry, 3 * char_w + 4, row_h);
                }
                fl_color(ev.note == 255 ? 80 : 255, ev.note == 255 ? 80 : 255, ev.note == 255 ? 80 : 255);
                if (ev.note == 255) fl_draw("---", col_x, ry + 14);
                else {
                    const char* notes[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};
                    char buf[8]; snprintf(buf, 8, "%s%d", notes[ev.note % 12], ev.note / 12);
                    fl_draw(buf, col_x, ry + 14);
                }
                col_x += 4 * char_w;

                // Sample
                if ((int)r == m_cursor_row && (int)t == m_cursor_track && (int)c == m_cursor_col && m_cursor_field == 1) {
                    fl_color(100, 100, 150); fl_rectf(col_x - 1, ry, 2 * char_w + 4, row_h);
                }
                fl_color(0, is_sampler ? 200 : 40, is_sampler ? 200 : 40);
                if (ev.sample_idx == 0) fl_draw("..", col_x, ry + 14);
                else { char buf[4]; snprintf(buf, 4, "%02X", ev.sample_idx); fl_draw(buf, col_x, ry + 14); }
                col_x += 3 * char_w;

                // Volume
                if ((int)r == m_cursor_row && (int)t == m_cursor_track && (int)c == m_cursor_col && m_cursor_field == 2) {
                    fl_color(100, 100, 150); fl_rectf(col_x - 1, ry, 2 * char_w + 4, row_h);
                }
                fl_color(0, 255, 0);
                if (ev.volume == 255) fl_draw("..", col_x, ry + 14);
                else { char buf[4]; snprintf(buf, 4, "%02X", ev.volume); fl_draw(buf, col_x, ry + 14); }
                col_x += 3 * char_w;
            }

            const auto& row_ev = m_pattern.event(t, r, 0); 
            
            // FX1
            if ((int)r == m_cursor_row && (int)t == m_cursor_track && m_cursor_field == 3) {
                fl_color(100, 100, 150); fl_rectf(col_x - 1, ry, 4 * char_w + 4, row_h);
            }
            fl_color(255, 255, 0);
            char fx1[8]; snprintf(fx1, 8, "%02X%02X", row_ev.effect1, row_ev.param1); fl_draw(fx1, col_x, ry + 14);
            col_x += 5 * char_w;
            
            // FX2
            if ((int)r == m_cursor_row && (int)t == m_cursor_track && m_cursor_field == 4) {
                fl_color(100, 100, 150); fl_rectf(col_x - 1, ry, 4 * char_w + 4, row_h);
            }
            char fx2[8]; snprintf(fx2, 8, "%02X%02X", row_ev.effect2, row_ev.param2); fl_draw(fx2, col_x, ry + 14);
        }
        fl_color(50, 50, 50); fl_line(cur_x + track_w, y(), cur_x + track_w, y() + h());
        cur_x += track_w + 10;
    }
    fl_pop_clip();
}

int TrackerView::handle(int event) {
    switch (event) {
        case FL_FOCUS:
        case FL_UNFOCUS:
            redraw();
            return 1;
        case FL_PUSH: {
            take_focus();
            int mx = Fl::event_x(), my = Fl::event_y();
            if (my >= y() && my < y() + 20) { // Header
                for (size_t t = 0; t < m_track_ui.size(); ++t) {
                    const auto& ui = m_track_ui[t];
                    if (mx >= ui.btn_plus_x && mx < ui.btn_plus_x + 18) { m_pattern.set_column_count(t, m_pattern.column_count(t) + 1); redraw(); return 1; }
                    if (mx >= ui.btn_minus_x && mx < ui.btn_minus_x + 18) { size_t current = m_pattern.column_count(t); if (current > 1) { m_pattern.set_column_count(t, current - 1); redraw(); } return 1; }
                }
            } else { // Grid
                m_cursor_row = std::min((int)m_pattern.row_count()-1, std::max(0, (my - y() - 20) / 18));
                int tx = x() + 40;
                for (size_t t = 0; t < m_track_ui.size(); ++t) {
                    if (mx >= m_track_ui[t].x && mx < m_track_ui[t].x + m_track_ui[t].w) { m_cursor_track = (int)t; break; }
                }
                if (Fl::event_state(FL_SHIFT)) {
                    if (!m_sel_active) { m_sel_active = true; m_sel_start_row = m_cursor_row; m_sel_start_track = m_cursor_track; }
                    m_sel_end_row = m_cursor_row; m_sel_end_track = m_cursor_track;
                } else { m_sel_active = false; }
                redraw();
            }
            return 1;
        }
        case FL_KEYUP: {
             Action action = m_engine.m_key_bindings.get_action(Fl::event_key(), Fl::event_state() & (FL_CTRL | FL_SHIFT | FL_ALT | FL_META));
             auto is_note_action = [](Action a) -> bool {
                return (int)a >= (int)Action::NoteC && (int)a <= (int)Action::NoteB;
             };
             if (is_note_action(action)) {
                 m_engine.stop_preview(m_cursor_track);
                 return 1;
             }
             return 0;
        }
        case FL_DRAG: {
            int my = Fl::event_y(), mx = Fl::event_x();
            m_cursor_row = std::min((int)m_pattern.row_count()-1, std::max(0, (my - y() - 20) / 18));
            for (size_t t = 0; t < m_track_ui.size(); ++t) {
                if (mx >= m_track_ui[t].x && mx < m_track_ui[t].x + m_track_ui[t].w) { m_cursor_track = (int)t; break; }
            }
            if (!m_sel_active) { m_sel_active = true; m_sel_start_row = m_cursor_row; m_sel_start_track = m_cursor_track; }
            m_sel_end_row = m_cursor_row; m_sel_end_track = m_cursor_track;
            redraw();
            return 1;
        }
        case FL_KEYDOWN: {
            int key = Fl::event_key();
            bool shift = Fl::event_state(FL_SHIFT);
            
            Action action = m_engine.m_key_bindings.get_action(key, Fl::event_state() & (FL_CTRL | FL_SHIFT | FL_ALT | FL_META));
            
            // Note mapping helper
            auto action_to_note = [](Action a) -> int {
                switch(a) {
                    case Action::NoteC: return 0;  case Action::NoteCs: return 1;
                    case Action::NoteD: return 2;  case Action::NoteDs: return 3;
                    case Action::NoteE: return 4;  case Action::NoteF: return 5;
                    case Action::NoteFs: return 6; case Action::NoteG: return 7;
                    case Action::NoteGs: return 8; case Action::NoteA: return 9;
                    case Action::NoteAs: return 10; case Action::NoteB: return 11;
                    default: return -1;
                }
            };

            int semitone = action_to_note(action);
            if (semitone >= 0) {
                int final_note = semitone + (m_engine.base_octave() * 12);
                if (m_engine.m_record_enabled) {
                    insert_note(final_note);
                } else {
                    m_engine.preview_note(m_cursor_track, final_note);
                }
                return 1;
            }

            if (shift && !m_sel_active) { m_sel_active = true; m_sel_start_row = m_cursor_row; m_sel_start_track = m_cursor_track; }
            
            int num_fields = 3 + 2; // Note, Sample, Vol, FX1, FX2
            size_t num_cols = m_pattern.column_count(m_cursor_track);

            switch (key) {
                case FL_Up: m_cursor_row = std::max(0, m_cursor_row - 1); break;
                case FL_Down: m_cursor_row = std::min((int)m_pattern.row_count() - 1, m_cursor_row + 1); break;
                case FL_Left: 
                    m_cursor_field--;
                    if (m_cursor_field < 0) {
                        m_cursor_field = num_fields - 1;
                        m_cursor_col--;
                        if (m_cursor_col < 0) {
                            if (m_cursor_track > 0) {
                                m_cursor_track--;
                                m_cursor_col = (int)m_pattern.column_count(m_cursor_track) - 1;
                            } else {
                                m_cursor_col = 0;
                                m_cursor_field = 0;
                            }
                        }
                    }
                    break;
                case FL_Right: 
                    m_cursor_field++;
                    if (m_cursor_field >= num_fields) {
                        m_cursor_field = 0;
                        m_cursor_col++;
                        if (m_cursor_col >= (int)num_cols) {
                            if (m_cursor_track < (int)m_engine.track_count() - 1) {
                                m_cursor_track++;
                                m_cursor_col = 0;
                            } else {
                                m_cursor_col = (int)num_cols - 1;
                                m_cursor_field = num_fields - 1;
                            }
                        }
                    }
                    break;
                case FL_Page_Up: m_cursor_row = std::max(0, m_cursor_row - 16); break;
                case FL_Page_Down: m_cursor_row = std::min((int)m_pattern.row_count() - 1, m_cursor_row + 16); break;
                case FL_Home: m_cursor_row = 0; break;
                case FL_End: m_cursor_row = (int)m_pattern.row_count() - 1; break;
                case FL_Escape: m_sel_active = false; break;
            }
            if (shift) { m_sel_end_row = m_cursor_row; m_sel_end_track = m_cursor_track; }
            else if (key != FL_Shift_L && key != FL_Shift_R) { m_sel_active = false; }
            redraw();
            return 1;
        }
    }
    return Fl_Widget::handle(event);
}

void TrackerView::set_current_row(int row) { m_cursor_row = row; redraw(); }
void TrackerView::insert_note(uint8_t note) {
    if (m_cursor_track < (int)m_engine.track_count()) {
        m_pattern.event(m_cursor_track, m_cursor_row, m_cursor_col).note = note;
        m_cursor_row = std::min((int)m_pattern.row_count()-1, m_cursor_row + 1);
        redraw();
    }
}

void TrackerView::recalculate_size() {
    int char_w = 8;
    int row_h = 18;
    size_t num_tracks = m_engine.track_count();
    size_t num_rows = m_pattern.row_count();
    
    int total_w = 40; // row numbers
    for (size_t t = 0; t < num_tracks; ++t) {
        size_t num_cols = m_pattern.column_count(t);
        total_w += (int)(num_cols * 10 * char_w + 2 * 4 * char_w + 40 + 10);
    }
    
    int total_h = 20 + (int)num_rows * row_h + 20;
    
    if (parent()) {
        if (parent()->w() > total_w) total_w = parent()->w();
        if (parent()->h() > total_h) total_h = parent()->h();
    }
    
    if (total_w != w() || total_h != h()) {
        // Use Fl_Widget::resize directly to avoid recursion if called from draw
        // Actually, in FLTK, resize() doesn't usually call draw() immediately, but marks for redraw.
        // But let's be careful.
        this->size(total_w, total_h);
    }
}

} // namespace disgrace_ns
