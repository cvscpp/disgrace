/*
 * Disgrace - Digital Audio Workstation
 * Copyright (C) 2025  Miroslav Shaltev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "tracks_panel.h"
#include "../core/engine.h"
#include "../instrument/sample_instrument.h"
#include "../instrument/soundfont_instrument.h"
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <algorithm>
#include <cmath>

namespace disgrace_ns {

TracksView::TracksView(int x, int y, int w, int h, Engine& engine)
    : Fl_Widget(x, y, w, h), m_engine(engine) {
    m_zoom = 10.0; // Default zoom
}

int TracksView::get_total_ticks() {
    auto order = m_engine.order_list();
    int total_rows = 0;
    for (auto pat_idx : order) {
        total_rows += (int)m_engine.pattern(pat_idx).row_count();
    }
    return total_rows;
}

int TracksView::tick_to_x(int tick) {
    return (int)(tick * m_zoom);
}

int TracksView::x_to_tick(int x) {
    if (m_zoom <= 0) return 0;
    return (int)(x / m_zoom);
}

static void draw_waveform_helper(int x, int y, int w, int h, const SampleData& data, Fl_Color col) {
    if (data.left.empty() || w <= 0) return;
    fl_color(col);
    int mid_y = y + h / 2;
    double samples_per_pixel = (double)data.left.size() / w;
    for (int i = 0; i < w; ++i) {
        size_t start = (size_t)(i * samples_per_pixel);
        size_t end = (size_t)((i + 1) * samples_per_pixel);
        if (end > data.left.size()) end = data.left.size();
        if (start >= end) {
            if (start < data.left.size()) {
                int amp = (int)(data.left[start] * (h / 2 - 2));
                fl_line(x + i, mid_y - amp, x + i, mid_y + amp);
            }
            continue;
        }
        float min_v = 1.0f, max_v = -1.0f;
        for (size_t s = start; s < end; ++s) {
            if (data.left[s] < min_v) min_v = data.left[s];
            if (data.left[s] > max_v) max_v = data.left[s];
        }
        int y1 = mid_y + (int)(min_v * (h / 2 - 2));
        int y2 = mid_y + (int)(max_v * (h / 2 - 2));
        fl_line(x + i, y1, x + i, y2);
    }
}

void TracksView::draw() {
    fl_push_clip(x(), y(), w(), h());
    
    // Background
    fl_color((Fl_Color)m_engine.m_tracker_bg);
    fl_rectf(x(), y(), w(), h());

    int track_h = 80;
    int header_w = 120;
    int num_tracks = (int)m_engine.track_count();
    auto order = m_engine.order_list();
    uint32_t lpb = m_engine.lpb();

    // Draw Time Scale (Header)
    fl_color((Fl_Color)m_engine.m_tracker_lpb_highlight);
    fl_rectf(x() + header_w, y(), w() - header_w, 20);
    fl_color((Fl_Color)m_engine.m_tracker_text);
    fl_font(FL_HELVETICA, 10);

    int total_rows = 0;
    for (size_t i = 0; i < order.size(); ++i) {
        auto& pat = m_engine.pattern(order[i]);
        int pat_rows = (int)pat.row_count();
        int px = x() + header_w + tick_to_x(total_rows);
        
        // Pattern boundary
        fl_color((Fl_Color)m_engine.m_tracker_lpb_highlight);
        fl_line(px, y(), px, y() + h());
        
        // Pattern label
        char buf[32]; snprintf(buf, 32, "POS %zu (PAT %d)", i, order[i]);
        fl_color((Fl_Color)m_engine.m_tracker_text);
        fl_draw(buf, px + 5, y() + 15);

        // Beat markers
        if (lpb > 0) {
            for (int r = 0; r < pat_rows; r += lpb) {
                int bx = px + tick_to_x(r);
                fl_color((Fl_Color)m_engine.m_tracker_lpb_highlight);
                fl_line(bx, y() + 15, bx, y() + 25);
                if (r % (lpb * 4) == 0) {
                    char bbuf[16]; snprintf(bbuf, 16, "%d", r / lpb);
                    fl_draw(bbuf, bx + 2, y() + 28);
                }
            }
        }
        total_rows += pat_rows;
    }

    // Draw Tracks
    for (int t = 0; t < num_tracks; ++t) {
        int ty = y() + 30 + t * track_h;
        if (ty > y() + h()) break;

        // Track Header
        fl_color((Fl_Color)m_engine.m_bg_color);
        fl_rectf(x(), ty, header_w, track_h - 1);
        fl_color((Fl_Color)m_engine.m_fg_color);
        fl_rect(x(), ty, header_w, track_h - 1);
        
        auto& track_obj = m_engine.track(t);
        fl_font(FL_HELVETICA_BOLD, 12);
        fl_draw(track_obj.name().substr(0, 15).c_str(), x() + 5, ty + 20);
        
        // Instrument Info
        fl_font(FL_HELVETICA, 10);
        Instrument* inst = track_obj.instrument();
        if (inst) {
            fl_draw(inst->name().substr(0, 20).c_str(), x() + 5, ty + 35);
            const char* type_str = "";
            switch(inst->type()) {
                case InstrumentType::Sampler: type_str = "[Sampler]"; break;
                case InstrumentType::SoundFont: type_str = "[SoundFont]"; break;
                case InstrumentType::Plugin: type_str = "[Plugin]"; break;
                case InstrumentType::Midi: type_str = "[MIDI]"; break;
                default: type_str = "[None]"; break;
            }
            fl_draw(type_str, x() + 5, ty + 50);
        }

        // Track Content Area
        int rows_done = 0;
        for (auto pat_idx : order) {
            auto& pat = m_engine.pattern(pat_idx);
            int pat_rows = (int)pat.row_count();
            int px = x() + header_w + tick_to_x(rows_done);

            size_t num_cols = pat.column_count(t);
            for (int r = 0; r < pat_rows; ++r) {
                for (size_t c = 0; c < num_cols; ++c) {
                    const auto& ev = pat.event(t, r, c);
                    if (ev.note != 255) {
                        int nx = px + tick_to_x(r);
                        
                        if (ev.note == 254) { // Note Off
                            fl_color(255, 100, 100);
                            fl_line(nx, ty + 5, nx, ty + track_h - 5);
                        } else {
                            // Find note length (scan ahead for next note or note off in this column)
                            int note_len = 1;
                            bool found_end = false;
                            // This is a simplified length finding, only within current pattern
                            for (int r2 = r + 1; r2 < pat_rows; ++r2) {
                                if (pat.event(t, r2, c).note != 255) {
                                    note_len = r2 - r;
                                    found_end = true;
                                    break;
                                }
                            }
                            if (!found_end) note_len = pat_rows - r; // To end of pattern for now

                            int nw = tick_to_x(note_len);
                            if (nw < 2) nw = 2;

                            if (inst && inst->type() == InstrumentType::Sampler) {
                                SampleInstrument* sampler = static_cast<SampleInstrument*>(inst);
                                size_t s_idx = (ev.sample_idx > 0) ? (ev.sample_idx - 1) : sampler->selected_sample();
                                if (s_idx < sampler->sample_count()) {
                                    auto& sample = sampler->get_sample(s_idx);
                                    if (sample.data) {
                                        // Draw sample background
                                        fl_color((Fl_Color)m_engine.m_tracker_lpb_highlight);
                                        fl_rectf(nx, ty + 5, nw, track_h - 10);
                                        // Draw waveform
                                        draw_waveform_helper(nx, ty + 5, nw, track_h - 10, *sample.data, (Fl_Color)m_engine.m_tracker_note);
                                    }
                                }
                            } else {
                                // Just a block for SoundFont or others
                                fl_color((Fl_Color)m_engine.m_tracker_note);
                                fl_rectf(nx, ty + 5, nw, track_h - 10);
                                fl_color((Fl_Color)m_engine.m_tracker_bg);
                                fl_font(FL_HELVETICA, 8);
                                const char* notes[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};
                                char nbuf[16]; snprintf(nbuf, 16, "%s%d", notes[ev.note % 12], ev.note / 12);
                                if (nw > 20) fl_draw(nbuf, nx + 2, ty + 15);
                            }
                        }
                    }
                }
            }
            rows_done += pat_rows;
        }

        // Horizontal line between tracks
        fl_color((Fl_Color)m_engine.m_tracker_lpb_highlight);
        fl_line(x() + header_w, ty + track_h - 1, x() + header_w + tick_to_x(total_rows), ty + track_h - 1);
    }

    // Current Playback Marker
    if (m_engine.transport_state() != TransportState::Stopped) {
        // We need global row for song play. 
        // Engine::m_current_row is updated in process_tick.
        int play_tick = (int)m_engine.m_current_row; 
        int play_x = x() + header_w + tick_to_x(play_tick);
        fl_color(255, 255, 255);
        fl_line(play_x, y(), play_x, y() + h());
    }

    // Selection
    if (m_sel_start_tick != -1 && m_sel_end_tick != -1) {
        int s1 = std::min(m_sel_start_tick, m_sel_end_tick);
        int s2 = std::max(m_sel_start_tick, m_sel_end_tick);
        int sx1 = x() + header_w + tick_to_x(s1);
        int sx2 = x() + header_w + tick_to_x(s2);
        fl_color(FL_SELECTION_COLOR);
        // Draw semi-transparent selection if supported, or just a box
        fl_line(sx1, y() + 20, sx1, y() + h());
        fl_line(sx2, y() + 20, sx2, y() + h());
        // fl_rectf(sx1, y() + 20, sx2 - sx1, h() - 20); // This hides everything under it
    }

    fl_pop_clip();
}

int TracksView::handle(int event) {
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
        case FL_MOUSEWHEEL:
            if (Fl::event_state(FL_CTRL)) {
                if (Fl::event_dy() < 0) zoom_in();
                else zoom_out();
                return 1;
            }
            break;
    }
    return Fl_Widget::handle(event);
}

void TracksView::zoom_in() { m_zoom *= 1.5; update_view(); }
void TracksView::zoom_out() { m_zoom /= 1.5; if (m_zoom < 0.1) m_zoom = 0.1; update_view(); }
void TracksView::view_all() {
    int total = get_total_ticks();
    if (total > 0) {
        m_zoom = (double)(parent()->w() - 140) / total;
        if (m_zoom < 0.1) m_zoom = 0.1;
    }
    update_view();
}
void TracksView::view_selection() {
    if (m_sel_start_tick != -1 && m_sel_end_tick != -1) {
        int diff = std::abs(m_sel_end_tick - m_sel_start_tick);
        if (diff > 0) {
            m_zoom = (double)(parent()->w() - 140) / diff;
        }
    }
    update_view();
}

void TracksView::update_view() {
    int total_w = 120 + tick_to_x(get_total_ticks()) + 50;
    int total_h = 30 + (int)m_engine.track_count() * 80 + 50;
    size(total_w, total_h);
    if (parent()) parent()->redraw();
    redraw();
}

// TracksPanel Implementation
TracksPanel::TracksPanel(int x, int y, int w, int h, Engine& engine)
    : Fl_Group(x, y, w, h), m_engine(engine) {
    
    begin();
    
    int btn_w = 80;
    int btn_h = 25;
    int cur_x = x + 5;
    
    m_zoom_in_btn = new Fl_Button(cur_x, y + 5, btn_w, btn_h, "Zoom In");
    m_zoom_in_btn->callback(cb_zoom_in, this);
    cur_x += btn_w + 5;
    
    m_zoom_out_btn = new Fl_Button(cur_x, y + 5, btn_w, btn_h, "Zoom Out");
    m_zoom_out_btn->callback(cb_zoom_out, this);
    cur_x += btn_w + 5;
    
    m_view_all_btn = new Fl_Button(cur_x, y + 5, btn_w, btn_h, "View All");
    m_view_all_btn->callback(cb_view_all, this);
    cur_x += btn_w + 5;
    
    m_view_sel_btn = new Fl_Button(cur_x, y + 5, btn_w, btn_h, "View Sel");
    m_view_sel_btn->callback(cb_view_sel, this);
    
    m_scroll = new Fl_Scroll(x, y + 35, w, h - 35);
    m_scroll->type(Fl_Scroll::BOTH);
    
    m_tracks_view = new TracksView(x, y + 35, w - 20, h - 55, m_engine);
    
    m_scroll->end();
    
    end();
    
    m_tracks_view->view_all();
}

void TracksPanel::update() {
    m_tracks_view->redraw();
}

void TracksPanel::cb_zoom_in(Fl_Widget*, void* d) { static_cast<TracksPanel*>(d)->m_tracks_view->zoom_in(); }
void TracksPanel::cb_zoom_out(Fl_Widget*, void* d) { static_cast<TracksPanel*>(d)->m_tracks_view->zoom_out(); }
void TracksPanel::cb_view_all(Fl_Widget*, void* d) { static_cast<TracksPanel*>(d)->m_tracks_view->view_all(); }
void TracksPanel::cb_view_sel(Fl_Widget*, void* d) { static_cast<TracksPanel*>(d)->m_tracks_view->view_selection(); }

} // namespace disgrace_ns
