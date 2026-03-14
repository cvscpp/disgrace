#include "notation_panel.h"
#include "../core/engine.h"
#include "../instrument/instrument.h"
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <algorithm>
#include <cmath>

namespace disgrace_ns {

NotationView::NotationView(int x, int y, int w, int h, Engine& engine)
    : Fl_Widget(x, y, w, h), m_engine(engine) {
    m_zoom = 25.0; 
}

int NotationView::get_total_ticks() {
    auto order = m_engine.order_list();
    int total_rows = 0;
    for (auto pat_idx : order) {
        total_rows += (int)m_engine.pattern(pat_idx).row_count();
    }
    return total_rows;
}

int NotationView::tick_to_x(int tick) { return (int)(tick * m_zoom); }
int NotationView::x_to_tick(int x) { return (m_zoom > 0) ? (int)(x / m_zoom) : 0; }

void NotationView::draw_staff(int tx, int ty, int tw, int type) {
    fl_color((Fl_Color)m_engine.m_tracker_text);
    int line_spacing = 8;
    
    auto draw_5_lines = [&](int start_y) {
        for (int i = 0; i < 5; ++i) {
            fl_line(tx, start_y + i * line_spacing, tx + tw, start_y + i * line_spacing);
        }
    };

    if (type == 0 || type == 3) { // Violin or Drums
        draw_5_lines(ty + 20);
        // Clef placeholder
        fl_font(FL_HELVETICA_BOLD, 20);
        fl_draw(type == 0 ? "&" : "||", tx + 5, ty + 45);
    } else if (type == 1) { // Bass
        draw_5_lines(ty + 20);
        fl_font(FL_HELVETICA_BOLD, 20);
        fl_draw("?", tx + 5, ty + 45); // Simplified Bass clef
    } else if (type == 2) { // Both
        draw_5_lines(ty + 10);
        draw_5_lines(ty + 10 + 6 * line_spacing);
        fl_font(FL_HELVETICA_BOLD, 16);
        fl_draw("&", tx + 5, ty + 30);
        fl_draw("?", tx + 5, ty + 30 + 6 * line_spacing);
    }
}

void NotationView::draw_note(int nx, int ny, int note, int staff_type) {
    // Very simplified pitch to Y mapping
    // Middle C (60)
    int line_spacing = 8;
    int base_y = ny + 20; 
    
    // Note names mapping to staff positions (0 = C, 1 = D, etc.)
    static const int pitch_to_pos[] = {0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6};
    static const bool is_sharp[] = {false, true, false, true, false, false, true, false, true, false, true, false};

    int octave = note / 12;
    int pitch = note % 12;
    int pos = pitch_to_pos[pitch] + (octave - 4) * 7;
    
    int dy = 0;
    if (staff_type == 0 || staff_type == 2 || staff_type == 3) { // Treble/Violin
        // Middle C (octave 4, pos 0) is one ledger line below staff (y = base_y + 5 * line_spacing)
        // Staff lines are at 0, 1, 2, 3, 4 * line_spacing
        // E4 (octave 4, pos 2) is at 4 * line_spacing
        dy = (10 - pos) * (line_spacing / 2);
    } else if (staff_type == 1) { // Bass
        // Middle C (octave 4, pos 0) is one ledger line above staff
        // A2 (octave 2, pos 5) is bottom line? No, G2 is bottom line.
        dy = (pos + 2) * (line_spacing / 2); // needs better mapping
    }

    int final_y = base_y + dy;
    
    fl_color((Fl_Color)m_engine.m_tracker_note);
    fl_pie(nx - 4, final_y - 3, 8, 6, 0, 360);
    fl_line(nx + 3, final_y, nx + 3, final_y - 20); // stem

    if (is_sharp[pitch]) {
        fl_font(FL_HELVETICA, 10);
        fl_draw("#", nx - 12, final_y + 4);
    }
    
    // Ledger lines
    if (dy >= 5 * line_spacing || dy < 0) {
        // ... draw ledger lines ...
    }
}

void NotationView::draw() {
    fl_push_clip(x(), y(), w(), h());
    fl_color((Fl_Color)m_engine.m_tracker_bg);
    fl_rectf(x(), y(), w(), h());

    int track_h = 120;
    int header_w = 120;
    int num_tracks = (int)m_engine.track_count();
    auto order = m_engine.order_list();
    uint32_t lpb = m_engine.lpb();

    int total_rows = get_total_ticks();
    int full_tw = tick_to_x(total_rows);

    // Draw Tracks
    for (int t = 0; t < num_tracks; ++t) {
        auto& track_obj = m_engine.track(t);
        Instrument* inst = track_obj.instrument();
        if (!inst || (inst->type() != InstrumentType::SoundFont && 
                      inst->type() != InstrumentType::Plugin && 
                      inst->type() != InstrumentType::Midi)) continue;

        int ty = y() + 30 + t * track_h;
        if (ty > y() + h()) break;

        // Track Header
        fl_color((Fl_Color)m_engine.m_bg_color);
        fl_rectf(x(), ty, header_w, track_h - 1);
        fl_color((Fl_Color)m_engine.m_fg_color);
        fl_font(FL_HELVETICA_BOLD, 12);
        fl_draw(track_obj.name().substr(0, 15).c_str(), x() + 5, ty + 20);

        // Draw Staff
        int staff_type = (int)track_obj.notation();
        draw_staff(x() + header_w, ty, full_tw, staff_type);

        // Draw Notes
        int rows_done = 0;
        for (auto pat_idx : order) {
            auto& pat = m_engine.pattern(pat_idx);
            int pat_rows = (int)pat.row_count();
            int px = x() + header_w + tick_to_x(rows_done);

            size_t num_cols = pat.column_count(t);
            for (int r = 0; r < pat_rows; ++r) {
                for (size_t c = 0; c < num_cols; ++c) {
                    const auto& ev = pat.event(t, r, c);
                    if (ev.note < 128) {
                        draw_note(px + tick_to_x(r), ty, ev.note, staff_type);
                    }
                }
            }
            rows_done += pat_rows;
        }
    }

    // Playback Marker
    if (m_engine.transport_state() != TransportState::Stopped) {
        int play_x = x() + header_w + tick_to_x((int)m_engine.m_current_row);
        fl_color(255, 255, 255);
        fl_line(play_x, y(), play_x, y() + h());
    }

    fl_pop_clip();
}

int NotationView::handle(int event) {
    int header_w = 120;
    switch(event) {
        case FL_PUSH:
            if (Fl::event_x() > x() + header_w) {
                m_is_selecting = true;
                m_sel_start_tick = x_to_tick(Fl::event_x() - (x() + header_w));
                m_sel_end_tick = m_sel_start_tick;
                redraw();
                return 1;
            }
            break;
        case FL_DRAG:
            if (m_is_selecting) {
                m_sel_end_tick = x_to_tick(Fl::event_x() - (x() + header_w));
                redraw();
                return 1;
            }
            break;
        case FL_RELEASE:
            m_is_selecting = false;
            return 1;
    }
    return Fl_Widget::handle(event);
}

void NotationView::zoom_in() { m_zoom *= 1.5; update_view(); }
void NotationView::zoom_out() { m_zoom /= 1.5; if (m_zoom < 1.0) m_zoom = 1.0; update_view(); }
void NotationView::view_all() {
    int total = get_total_ticks();
    if (total > 0) m_zoom = (double)(parent()->w() - 140) / total;
    update_view();
}
void NotationView::view_selection() {
    if (m_sel_start_tick != -1 && m_sel_end_tick != -1) {
        int diff = std::abs(m_sel_end_tick - m_sel_start_tick);
        if (diff > 0) m_zoom = (double)(parent()->w() - 140) / diff;
    }
    update_view();
}

void NotationView::update_view() {
    int total_w = 120 + tick_to_x(get_total_ticks()) + 50;
    int total_h = 30 + (int)m_engine.track_count() * 120 + 50;
    size(total_w, total_h);
    if (parent()) parent()->redraw();
    redraw();
}

NotationPanel::NotationPanel(int x, int y, int w, int h, Engine& engine)
    : Fl_Group(x, y, w, h), m_engine(engine) {
    begin();
    int btn_w = 80, btn_h = 25, cur_x = x + 5;
    m_zoom_in_btn = new Fl_Button(cur_x, y + 5, btn_w, btn_h, "Zoom In");
    m_zoom_in_btn->callback(cb_zoom_in, this); cur_x += btn_w + 5;
    m_zoom_out_btn = new Fl_Button(cur_x, y + 5, btn_w, btn_h, "Zoom Out");
    m_zoom_out_btn->callback(cb_zoom_out, this); cur_x += btn_w + 5;
    m_view_all_btn = new Fl_Button(cur_x, y + 5, btn_w, btn_h, "View All");
    m_view_all_btn->callback(cb_view_all, this); cur_x += btn_w + 5;
    m_view_sel_btn = new Fl_Button(cur_x, y + 5, btn_w, btn_h, "View Sel");
    m_view_sel_btn->callback(cb_view_sel, this);
    m_scroll = new Fl_Scroll(x, y + 35, w, h - 35);
    m_scroll->type(Fl_Scroll::BOTH);
    m_notation_view = new NotationView(x, y + 35, w - 20, h - 55, m_engine);
    m_scroll->end();
    end();
}

void NotationPanel::update() { m_notation_view->redraw(); }
void NotationPanel::cb_zoom_in(Fl_Widget*, void* d) { static_cast<NotationPanel*>(d)->m_notation_view->zoom_in(); }
void NotationPanel::cb_zoom_out(Fl_Widget*, void* d) { static_cast<NotationPanel*>(d)->m_notation_view->zoom_out(); }
void NotationPanel::cb_view_all(Fl_Widget*, void* d) { static_cast<NotationPanel*>(d)->m_notation_view->view_all(); }
void NotationPanel::cb_view_sel(Fl_Widget*, void* d) { static_cast<NotationPanel*>(d)->m_notation_view->view_selection(); }

} // namespace disgrace_ns
