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

#include <wx/app.h>
#include <wx/artprov.h>
#include <wx/dcclient.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include "wx_tracks_panel.h"
#include "wx_detached_frame.h"
#include "wx_main_window.h"
#include "../core/engine.h"
#include "../instrument/sample_instrument.h"
#include "../instrument/soundfont_instrument.h"
#include "../sequencer/pattern.h"
#include "../edit/cmd_track_cut.h"
#include "../edit/cmd_track_copy.h"
#include "../edit/cmd_track_paste.h"
#include "../edit/cmd_track_silence.h"
#include "../edit/cmd_track_insert_silence.h"
#include "wx_beat_quantize_dialog.h"
#include "theme.h"

namespace disgrace_ns {

static void draw_waveform_helper(wxDC& dc, int x, int y, int w, int h, const SampleData& data, const wxColour& col, size_t max_samples = 0) {
    if (data.left.empty() || w <= 0) return;
    dc.SetPen(wxPen(col));
    
    bool is_stereo = !data.right.empty();
    int ch_h = is_stereo ? h / 2 : h;

    size_t data_len = (max_samples > 0) ? std::min(max_samples, data.left.size()) : data.left.size();
    if (data_len == 0) return;

    auto draw_channel = [&](const std::vector<float>& ch_data, int ch_y) {
        int mid_y = ch_y + ch_h / 2;
        double samples_per_pixel = (double)data_len / w;
        for (int i = 0; i < w; ++i) {
            size_t start = (size_t)(i * samples_per_pixel);
            size_t end = (size_t)((i + 1) * samples_per_pixel);
            if (end > data_len) end = data_len;
            if (start >= end) {
                if (start < data_len) {
                    int amp = (int)(ch_data[start] * (ch_h / 2 - 2));
                    dc.DrawLine(x + i, mid_y - amp, x + i, mid_y + amp);
                }
                continue;
            }
            float min_v = 1.0f, max_v = -1.0f;
            for (size_t s = start; s < end; ++s) {
                if (ch_data[s] < min_v) min_v = ch_data[s];
                if (ch_data[s] > max_v) max_v = ch_data[s];
            }
            int y1 = mid_y + (int)(min_v * (ch_h / 2 - 2));
            int y2 = mid_y + (int)(max_v * (ch_h / 2 - 2));
            dc.DrawLine(x + i, y1, x + i, y2);
        }
    };

    draw_channel(data.left, y);
    if (is_stereo) {
        draw_channel(data.right, y + ch_h);
        // Draw a small divider line
        dc.SetPen(wxPen(wxColour(col.Red(), col.Green(), col.Blue(), 80)));
        dc.DrawLine(x, y + ch_h, x + w, y + ch_h);
        dc.SetPen(wxPen(col));
    }
}

enum {
    ID_ZOOM_IN = 10001,
    ID_ZOOM_OUT,
    ID_VIEW_ALL,
    ID_VIEW_SEL,
    ID_DETACH,
    ID_CUT,
    ID_COPY,
    ID_PASTE,
    ID_SILENCE,
    ID_INSERT_SILENCE,
    ID_MENU_CUT,
    ID_MENU_COPY,
    ID_MENU_PASTE,
    ID_MENU_SILENCE,
    ID_MENU_INSERT_SILENCE,
    ID_MENU_UNDO,
    ID_MENU_REDO,
    ID_BEAT_QUANTIZE,
    ID_MENU_BEAT_QUANTIZE,
    ID_MENU_CONVERT_SOUNDFONT,
    ID_MENU_CONVERT_SFZ,
    ID_MENU_CONVERT_PLUGIN
};

wxBEGIN_EVENT_TABLE(TracksPanel, wxPanel)
    EVT_BUTTON(ID_ZOOM_IN, TracksPanel::on_zoom_in)
    EVT_BUTTON(ID_ZOOM_OUT, TracksPanel::on_zoom_out)
    EVT_BUTTON(ID_VIEW_ALL, TracksPanel::on_view_all)
    EVT_BUTTON(ID_VIEW_SEL, TracksPanel::on_view_sel)
    EVT_BUTTON(ID_DETACH, TracksPanel::on_detach)
    EVT_BUTTON(ID_CUT, TracksPanel::on_cut)
    EVT_BUTTON(ID_COPY, TracksPanel::on_copy)
    EVT_BUTTON(ID_PASTE, TracksPanel::on_paste)
    EVT_BUTTON(ID_SILENCE, TracksPanel::on_silence)
    EVT_BUTTON(ID_INSERT_SILENCE, TracksPanel::on_insert_silence)
    EVT_BUTTON(ID_BEAT_QUANTIZE, TracksPanel::on_beat_quantize)
wxEND_EVENT_TABLE()

TracksPanel::TracksPanel(wxWindow* parent, Engine& engine)
    : wxPanel(parent, wxID_ANY), m_engine(engine)
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* btn_sizer = new wxBoxSizer(wxHORIZONTAL);
    int btn_h = 28;

    m_zoom_in_btn = new wxButton(this, ID_ZOOM_IN, "Zoom In", wxDefaultPosition, wxSize(-1, btn_h));
    m_zoom_in_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_PLUS, wxART_BUTTON, wxSize(16, 16)));
    m_zoom_out_btn = new wxButton(this, ID_ZOOM_OUT, "Zoom Out", wxDefaultPosition, wxSize(-1, btn_h));
    m_zoom_out_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_MINUS, wxART_BUTTON, wxSize(16, 16)));
    m_view_all_btn = new wxButton(this, ID_VIEW_ALL, "View All", wxDefaultPosition, wxSize(-1, btn_h));
    m_view_all_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_HOME, wxART_BUTTON, wxSize(16, 16)));
    m_view_sel_btn = new wxButton(this, ID_VIEW_SEL, "View Sel", wxDefaultPosition, wxSize(-1, btn_h));
    m_view_sel_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_FIND, wxART_BUTTON, wxSize(16, 16)));

    m_cut_btn = new wxButton(this, ID_CUT, "Cut", wxDefaultPosition, wxSize(-1, btn_h));
    m_cut_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_CUT, wxART_BUTTON, wxSize(16, 16)));
    m_copy_btn = new wxButton(this, ID_COPY, "Copy", wxDefaultPosition, wxSize(-1, btn_h));
    m_copy_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY, wxART_BUTTON, wxSize(16, 16)));
    m_paste_btn = new wxButton(this, ID_PASTE, "Paste", wxDefaultPosition, wxSize(-1, btn_h));
    m_paste_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE, wxART_BUTTON, wxSize(16, 16)));
    m_silence_btn = new wxButton(this, ID_SILENCE, "Silence", wxDefaultPosition, wxSize(-1, btn_h));
    m_silence_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_CROSS_MARK, wxART_BUTTON, wxSize(16, 16)));
    m_insert_btn = new wxButton(this, ID_INSERT_SILENCE, "Insert", wxDefaultPosition, wxSize(-1, btn_h));
    m_insert_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_PLUS, wxART_BUTTON, wxSize(16, 16)));

    m_beat_quantize_btn = new wxButton(this, ID_BEAT_QUANTIZE, "Beat Quantize", wxDefaultPosition, wxSize(-1, btn_h));
    m_beat_quantize_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE, wxART_BUTTON, wxSize(16, 16)));
    m_beat_quantize_btn->SetToolTip("Quantize selected audio region to beat grid");

    m_detach_btn = new wxButton(this, ID_DETACH, "", wxDefaultPosition, wxSize(btn_h, btn_h));
    m_detach_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_FULL_SCREEN, wxART_BUTTON, wxSize(16, 16)));
    m_detach_btn->SetToolTip("Detach / re-attach tracks view");

    btn_sizer->Add(m_zoom_in_btn, 0, wxALL, 2);
    btn_sizer->Add(m_zoom_out_btn, 0, wxALL, 2);
    btn_sizer->Add(m_view_all_btn, 0, wxALL, 2);
    btn_sizer->Add(m_view_sel_btn, 0, wxALL, 2);
    btn_sizer->AddStretchSpacer();
    btn_sizer->Add(m_cut_btn, 0, wxALL, 2);
    btn_sizer->Add(m_copy_btn, 0, wxALL, 2);
    btn_sizer->Add(m_paste_btn, 0, wxALL, 2);
    btn_sizer->Add(m_silence_btn, 0, wxALL, 2);
    btn_sizer->Add(m_insert_btn, 0, wxALL, 2);
    btn_sizer->Add(m_beat_quantize_btn, 0, wxALL, 2);
    btn_sizer->Add(m_detach_btn, 0, wxALL, 2);

    main_sizer->Add(btn_sizer, 0, wxEXPAND | wxALL, 2);

    m_tracks_view = new TracksView(this, wxID_ANY, m_engine);
    main_sizer->Add(m_tracks_view, 1, wxEXPAND | wxALL, 0);

    SetSizer(main_sizer);
}

void TracksPanel::update() {
    static int last_total_ticks = -1;
    static int last_track_count = -1;
    static size_t last_play_row = (size_t)-1;
    int current_total = m_tracks_view->get_total_ticks();
    int current_tracks = (int)m_engine.track_count();
    size_t current_row = m_engine.current_row();
    bool structural_change = (current_total != last_total_ticks || current_tracks != last_track_count);
    bool position_change   = (current_row != last_play_row);
    if (structural_change) {
        m_tracks_view->update_view();
        last_total_ticks = current_total;
        last_track_count = current_tracks;
    }
    if (structural_change || position_change) {
        last_play_row = current_row;
        m_tracks_view->Refresh();
    }
}

void TracksPanel::on_zoom_in(wxCommandEvent& event) { m_tracks_view->zoom_in(); }
void TracksPanel::on_zoom_out(wxCommandEvent& event) { m_tracks_view->zoom_out(); }
void TracksPanel::on_view_all(wxCommandEvent& event) { m_tracks_view->view_all(); }
void TracksPanel::on_view_sel(wxCommandEvent& event) { m_tracks_view->view_selection(); }
void TracksPanel::on_cut(wxCommandEvent& event) { m_tracks_view->do_cut(); }
void TracksPanel::on_copy(wxCommandEvent& event) { m_tracks_view->do_copy(); }
void TracksPanel::on_paste(wxCommandEvent& event) { m_tracks_view->do_paste(); }
void TracksPanel::on_silence(wxCommandEvent& event) { m_tracks_view->do_silence(); }
void TracksPanel::on_insert_silence(wxCommandEvent& event) { m_tracks_view->do_insert_silence(); }
void TracksPanel::on_beat_quantize(wxCommandEvent& event) { m_tracks_view->do_beat_quantize(); }
void TracksPanel::on_detach(wxCommandEvent& event) {
    if (m_detached_frame) {
        return;
    }
    Hide();
    m_detached_frame = new DetachedFrame(this, "Tracks", GetParent(), m_tab_index);
    m_detached_frame->set_on_detach_callback([this]() { m_detached_frame = nullptr; });
}

wxBEGIN_EVENT_TABLE(TracksView, wxScrolledWindow)
    EVT_PAINT(TracksView::OnPaint)
    EVT_SIZE(TracksView::OnSize)
    EVT_LEFT_DOWN(TracksView::OnMouseDown)
    EVT_MOTION(TracksView::OnMouseDrag)
    EVT_LEFT_UP(TracksView::OnMouseUp)
    EVT_MOUSEWHEEL(TracksView::OnMouseWheel)
    EVT_RIGHT_DOWN(TracksView::OnMouseRightClick)
    EVT_KEY_DOWN(TracksView::OnKeyDown)
    EVT_CONTEXT_MENU(TracksView::OnContextMenu)
wxEND_EVENT_TABLE()

TracksView::TracksView(wxWindow* parent, wxWindowID id, Engine& engine)
    : wxScrolledWindow(parent, id), m_engine(engine)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    m_zoom = 10.0;
    SetScrollRate(1, 1);
    m_needs_initial_view_all = true;
}

void TracksView::OnSize(wxSizeEvent& event) {
    if (m_needs_initial_view_all && GetClientSize().x > 150) {
        view_all();
    }
    event.Skip();
}

int TracksView::get_total_ticks() {
    auto order = m_engine.order_list();
    int total_rows = 0;
    for (auto pat_idx : order) {
        total_rows += (int)m_engine.pattern(pat_idx).row_count();
    }
    return total_rows;
}

int TracksView::tick_to_x(int tick) { return (int)(tick * m_zoom); }
int TracksView::x_to_tick(int x) { return (m_zoom > 0) ? (int)((double)x / m_zoom + 0.5) : 0; }

int TracksView::get_track_height(int track_idx) const {
    if (track_idx < 0 || (size_t)track_idx >= m_engine.track_count()) {
        return 80;
    }
    auto& track = m_engine.track(track_idx);
    return track.is_minimized() ? 20 : 80;
}

void TracksView::toggle_track_minimize(int track_idx) {
    if (track_idx < 0 || (size_t)track_idx >= m_engine.track_count()) {
        return;
    }
    auto& track = m_engine.track(track_idx);
    track.set_minimized(!track.is_minimized());
    update_view();
    Refresh();
}

int TracksView::hit_test_track(int client_y) const {
    int cur_y = 30;
    for (int t = 0; t < (int)m_engine.track_count(); ++t) {
        int track_h = get_track_height(t);
        if (client_y >= cur_y && client_y < cur_y + track_h)
            return t;
        cur_y += track_h;
    }
    return -1;
}

bool TracksView::can_convert_selected_track() const {
    if (m_selected_track < 0 || (size_t)m_selected_track >= m_engine.track_count())
        return false;

    auto* sampler = dynamic_cast<SampleInstrument*>(m_engine.track(m_selected_track).instrument());
    if (!sampler || sampler->sample_count() == 0)
        return false;

    for (size_t i = 0; i < sampler->sample_count(); ++i) {
        const auto& sample = sampler->get_sample(i);
        if (sample.data && !sample.data->left.empty())
            return true;
    }
    return false;
}

void TracksView::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    PrepareDC(dc);
    draw(dc);
}

void TracksView::draw(wxDC& dc) {
    dc.SetBrush(wxBrush(ThemeManager::toWxColour(m_engine.m_tracker_bg)));
    dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_bg)));
    wxSize virtual_size = GetVirtualSize();
    dc.DrawRectangle(0, 0, virtual_size.GetWidth(), virtual_size.GetHeight());

    int track_h = 80;
    int header_w = 120;
    int num_tracks = (int)m_engine.track_count();
    auto order = m_engine.order_list();
    uint32_t lpb = m_engine.lpb();

    // Draw Time Scale (Header)
    dc.SetBrush(wxBrush(ThemeManager::toWxColour(m_engine.m_tracker_lpb_highlight)));
    dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_lpb_highlight)));
    dc.DrawRectangle(header_w, 0, virtual_size.GetWidth() - header_w, 20);

    dc.SetTextForeground(ThemeManager::toWxColour(m_engine.m_tracker_text));
    wxFont header_font(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(header_font);

    int total_rows = 0;
    size_t current_pos = m_engine.current_order_pos();
    int play_tick = 0;

    for (size_t i = 0; i < order.size(); ++i) {
        auto& pat = m_engine.pattern(order[i]);
        int pat_rows = (int)pat.row_count();
        int px = header_w + tick_to_x(total_rows);

        // Calculate playhead position
        if (i < current_pos) {
            play_tick += pat_rows;
        } else if (i == current_pos) {
            play_tick += (int)m_engine.current_row();
        }

        // Pattern boundary
        dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_lpb_highlight)));
        dc.DrawLine(px, 0, px, virtual_size.GetHeight());

        // Pattern label
        wxString buf;
        buf.Printf("POS %zu (PAT %zu)", i, order[i]);
        dc.SetTextForeground(ThemeManager::toWxColour(m_engine.m_tracker_text));
        dc.DrawText(buf, px + 5, 2);

        // Beat markers
        if (lpb > 0) {
            for (int r = 0; r < pat_rows; r += lpb) {
                int bx = px + tick_to_x(r);
                dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_lpb_highlight)));
                dc.DrawLine(bx, 15, bx, 25);
                if (r % (lpb * 4) == 0) {
                    wxString bbuf;
                    bbuf.Printf("%d", r / lpb);
                    dc.DrawText(bbuf, bx + 2, 28);
                }
            }
        }

        total_rows += pat_rows;
    }

    // Draw Tracks
    int cur_y = 30;
    double bpm = m_engine.tempo();
    double sample_rate = m_engine.sample_rate();
    double samples_per_beat = (sample_rate * 60.0) / bpm;
    double samples_per_row = (lpb > 0) ? (samples_per_beat / lpb) : 44100.0;

    for (int t = 0; t < num_tracks; ++t) {
        auto& track_obj = m_engine.track(t);
        int track_h_actual = track_obj.is_minimized() ? 20 : 80;
        int ty = cur_y;
        cur_y += track_h_actual;

        // Track Header
        dc.SetBrush(wxBrush(ThemeManager::toWxColour(m_engine.m_bg_color)));
        dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_fg_color)));
        dc.DrawRectangle(0, ty, header_w, track_h_actual - 1);

        // Minimize button (- for expanded, + for minimized)
        int btn_w = 20;
        int btn_h = 18;
        int btn_x = 5;
        int btn_y = ty + (track_h_actual - btn_h) / 2;
        
        dc.SetBrush(wxBrush(ThemeManager::toWxColour(m_engine.m_tracker_lpb_highlight)));
        dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_fg_color)));
        dc.DrawRectangle(btn_x, btn_y, btn_w, btn_h);
        
        dc.SetTextForeground(ThemeManager::toWxColour(m_engine.m_fg_color));
        wxFont btn_font(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
        dc.SetFont(btn_font);
        wxString btn_text = track_obj.is_minimized() ? "+" : "-";
        dc.DrawText(btn_text, btn_x + 6, btn_y + 2);

        wxFont bold_font(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
        dc.SetFont(bold_font);
        dc.SetTextForeground(ThemeManager::toWxColour(m_engine.m_fg_color));
        wxString name = track_obj.name().substr(0, 15);
        dc.DrawText(name, 30, ty + 5);

        Instrument* inst = track_obj.instrument();

        // Instrument Info (only shown when not minimized)
        if (!track_obj.is_minimized()) {
            wxFont normal_font(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
            dc.SetFont(normal_font);
            dc.SetTextForeground(ThemeManager::toWxColour(m_engine.m_tracker_text));
            if (inst) {
                wxString inst_name = inst->name().substr(0, 20);
                dc.DrawText(inst_name, 30, ty + 20);
                const char* type_str = "";
                switch(inst->type()) {
                    case InstrumentType::Sampler: type_str = "[Sampler]"; break;
                    case InstrumentType::SoundFont: type_str = "[SoundFont]"; break;
                    case InstrumentType::Plugin: type_str = "[Plugin]"; break;
                    case InstrumentType::Midi: type_str = "[MIDI]"; break;
                    default: type_str = "[None]"; break;
                }
                dc.DrawText(type_str, 30, ty + 35);
            }
        }

        // Track Content Area (only shown when not minimized)
        if (!track_obj.is_minimized()) {
            int rows_done = 0;
            for (auto pat_idx : order) {
                auto& pat = m_engine.pattern(pat_idx);
                int pat_rows = (int)pat.row_count();
                int px = header_w + tick_to_x(rows_done);

                size_t num_cols = pat.column_count(t);
                for (int r = 0; r < pat_rows; ++r) {
                    for (size_t c = 0; c < num_cols; ++c) {
                        const auto& ev = pat.event(t, r, c);
                        if (ev.note != 255) {
                            int nx = px + tick_to_x(r);

                            if (ev.note == 254) { // Note Off
                                dc.SetPen(wxPen(wxColour(255, 100, 100)));
                                dc.DrawLine(nx, ty + 5, nx, ty + track_h_actual - 5);
                            } else {
                                // Find note length
                                int note_len = 1;
                                bool found_end = false;
                                for (int r2 = r + 1; r2 < pat_rows; ++r2) {
                                    if (pat.event(t, r2, c).note != 255) {
                                        note_len = r2 - r;
                                        found_end = true;
                                        break;
                                    }
                                }
                                if (!found_end) note_len = pat_rows - r;

                                if (inst && inst->type() == InstrumentType::Sampler) {
                                    SampleInstrument* sampler = static_cast<SampleInstrument*>(inst);
                                    size_t s_idx = (ev.sample_idx > 0) ? (ev.sample_idx - 1) : sampler->selected_sample();
                                    if (s_idx < sampler->sample_count()) {
                                        auto& sample = sampler->get_sample(s_idx);
                                        if (sample.data) {
                                            double sample_duration_rows = (double)sample.data->left.size() / samples_per_row;
                                            int nw_limit = tick_to_x(note_len);
                                            int nw_sample = tick_to_x(sample_duration_rows);
                                            int nw = std::min(nw_limit, nw_sample);
                                            if (nw < 2) nw = 2;

                                            size_t samples_to_draw = (size_t)((double)nw / m_zoom * samples_per_row);
                                            samples_to_draw = std::min(samples_to_draw, sample.data->left.size());

                                            // Detect overlaps
                                            auto overlaps = detect_overlaps(sampler);
                                            bool is_overlapping = (s_idx < overlaps.size()) && overlaps[s_idx].is_overlapping;
                                            
                                            // Draw sample background
                                            if (is_overlapping) {
                                                dc.SetBrush(wxBrush(wxColour(255, 200, 0, 128)));
                                                dc.SetPen(wxPen(wxColour(255, 150, 0)));
                                            } else {
                                                dc.SetBrush(wxBrush(ThemeManager::toWxColour(m_engine.m_tracker_lpb_highlight)));
                                                dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_lpb_highlight)));
                                            }
                                            dc.DrawRectangle(nx, ty + 5, nw, track_h_actual - 10);
                                            
                                            // Draw waveform
                                            draw_waveform_helper(dc, nx, ty + 5, nw, track_h_actual - 10, *sample.data, ThemeManager::toWxColour(m_engine.m_tracker_note), samples_to_draw);
                                            
                                            // Draw overlap indicator if needed
                                            if (is_overlapping) {
                                                dc.SetPen(wxPen(wxColour(255, 100, 0), 2));
                                                dc.DrawRectangle(nx, ty + 5, nw, track_h_actual - 10);
                                            }
                                        }
                                    }
                                } else {
                                    int nw = tick_to_x(note_len);
                                    if (nw < 2) nw = 2;
                                    // Just a block
                                    dc.SetBrush(wxBrush(ThemeManager::toWxColour(m_engine.m_tracker_note)));
                                    dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_note)));
                                    dc.DrawRectangle(nx, ty + 5, nw, track_h_actual - 10);
                                    
                                    dc.SetTextForeground(ThemeManager::toWxColour(m_engine.m_tracker_bg));
                                    wxFont note_font(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
                                    dc.SetFont(note_font);
                                    const char* notes[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};
                                    wxString nbuf;
                                    nbuf.Printf("%s%d", notes[ev.note % 12], ev.note / 12);
                                    if (nw > 20) dc.DrawText(nbuf, nx + 2, ty + 15);
                                }
                            }
                        }
                    }
                }
                rows_done += pat_rows;
            }
        }

        // Horizontal line between tracks
        dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_lpb_highlight)));
        dc.DrawLine(header_w, ty + track_h_actual - 1, header_w + tick_to_x(total_rows), ty + track_h_actual - 1);
    }

    // Current Playback Marker
    if (m_engine.transport_state() != TransportState::Stopped) {
        int play_x = header_w + tick_to_x(play_tick);
        dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_cursor)));
        dc.DrawLine(play_x, 0, play_x, virtual_size.GetHeight());
    }

    // Selection - highlight on selected track AND guide on all tracks
    if (m_sel_start_tick != -1 && m_sel_end_tick != -1) {
        int s1 = std::min(m_sel_start_tick, m_sel_end_tick);
        int s2 = std::max(m_sel_start_tick, m_sel_end_tick);
        if (s1 == s2) s2 = s1 + 1;  // Minimum 1 tick width for visibility
        
        int sx1 = header_w + tick_to_x(s1);
        int sx2 = header_w + tick_to_x(s2);
        if (sx2 == sx1) sx2 = sx1 + 2;  // Minimum 2 pixels for visibility
        
        // Draw selection guide across all tracks
        wxColour sel_col = ThemeManager::toWxColour(m_engine.m_selection_color);
        dc.SetBrush(wxBrush(wxColour(sel_col.Red(), sel_col.Green(), sel_col.Blue(), 32)));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(sx1, 0, sx2 - sx1, virtual_size.GetHeight());

        // Draw selection highlight on the selected track if it's a sampler
        if (m_selected_track >= 0 && m_selected_track < num_tracks) {
            auto& sel_track = m_engine.track(m_selected_track);
            auto inst = sel_track.instrument();
            if (inst && inst->type() == InstrumentType::Sampler) {
                // Calculate track position
                int cur_y_pos = 30;
                for (int t = 0; t < m_selected_track; ++t) {
                    auto& t_obj = m_engine.track(t);
                    int t_h = t_obj.is_minimized() ? 20 : 80;
                    cur_y_pos += t_h;
                }
                
                int sel_track_h = sel_track.is_minimized() ? 20 : 80;
                
                // Draw selection highlight on this track
                dc.SetBrush(wxBrush(wxColour(sel_col.Red(), sel_col.Green(), sel_col.Blue(), 64)));
                dc.SetPen(wxPen(sel_col, 2));
                dc.DrawRectangle(sx1, cur_y_pos, sx2 - sx1, sel_track_h);
            }
        }
        
        // Border lines across all tracks
        dc.SetPen(wxPen(wxColour(sel_col.Red(), sel_col.Green(), sel_col.Blue(), 128), 1, wxPENSTYLE_DOT));
        dc.DrawLine(sx1, 0, sx1, virtual_size.GetHeight());
        dc.DrawLine(sx2, 0, sx2, virtual_size.GetHeight());
    }
}

void TracksView::OnMouseDown(wxMouseEvent& event) {
    int x, y;
    CalcUnscrolledPosition(event.GetX(), event.GetY(), &x, &y);
    int header_w = 120;
    
    // Determine which track was clicked
    int num_tracks = (int)m_engine.track_count();
    int cur_y = 30;
    int clicked_track = -1;
    
    for (int t = 0; t < num_tracks; ++t) {
        auto& track_obj = m_engine.track(t);
        int track_h = track_obj.is_minimized() ? 20 : 80;
        int ty = cur_y;
        
        if (y >= ty && y < ty + track_h) {
            clicked_track = t;
            
            // Check if minimize button was clicked (5-25 pixels from left, in header)
            if (x < header_w && x >= 5 && x <= 25) {
                toggle_track_minimize(t);
                return;
            }
            break;
        }
        cur_y += track_h;
    }
    
    // Track content selection - always set selected track even if not sampler
    if (x > header_w && clicked_track >= 0) {
        m_selected_track = clicked_track;
        m_is_selecting = true;
        m_sel_start_tick = x_to_tick(x - header_w);
        m_sel_end_tick = m_sel_start_tick;
        Refresh();
    }
}

void TracksView::OnMouseDrag(wxMouseEvent& event) {
    if (m_is_selecting) {
        int x, y;
        CalcUnscrolledPosition(event.GetX(), event.GetY(), &x, &y);
        int header_w = 120;
        m_sel_end_tick = x_to_tick(x - header_w);
        Refresh();
    }
}

void TracksView::OnMouseUp(wxMouseEvent& event) {
    m_is_selecting = false;
}

void TracksView::OnMouseWheel(wxMouseEvent& event) {
    if (event.ControlDown()) {
        if (event.GetWheelRotation() < 0) zoom_out();
        else zoom_in();
    } else {
        event.Skip();
    }
}

void TracksView::zoom_in() { m_zoom *= 1.5; update_view(); }
void TracksView::zoom_out() { m_zoom /= 1.5; if (m_zoom < 0.1) m_zoom = 0.1; update_view(); }
void TracksView::view_all() {
    int total = get_total_ticks();
    if (total > 0) {
        wxSize size = GetClientSize();
        if (size.GetWidth() <= 150) size = GetParent()->GetClientSize();
        if (size.GetWidth() > 150) {
            m_zoom = (double)(size.GetWidth() - 140) / total;
            if (m_zoom < 0.1) m_zoom = 0.1;
            m_needs_initial_view_all = false;
        }
    }
    update_view();
}
void TracksView::view_selection() {
    if (m_sel_start_tick != -1 && m_sel_end_tick != -1) {
        int diff = std::abs(m_sel_end_tick - m_sel_start_tick);
        if (diff > 0) {
            wxSize size = GetParent()->GetClientSize();
            m_zoom = (double)(size.GetWidth() - 140) / diff;
        }
    }
    update_view();
}

void TracksView::update_view() {
    int num_tracks = (int)m_engine.track_count();
    int total_h = 30;
    for (int t = 0; t < num_tracks; ++t) {
        total_h += get_track_height(t);
    }
    int total_w = 120 + tick_to_x(get_total_ticks()) + 50;
    total_h += 50;
    SetVirtualSize(total_w, total_h);
    Refresh();
}

void TracksView::OnMouseRightClick(wxMouseEvent& event) {
    int x, y;
    CalcUnscrolledPosition(event.GetX(), event.GetY(), &x, &y);
    int header_w = 120;
    if (x > header_w) {
        int clicked_track = hit_test_track(y);
        if (clicked_track >= 0) {
            m_selected_track = clicked_track;
            Refresh();
        }
    }
    event.Skip();
}

void TracksView::OnContextMenu(wxContextMenuEvent& event) {
    if (event.GetPosition() != wxDefaultPosition) {
        wxPoint client_pos = ScreenToClient(event.GetPosition());
        int x, y;
        CalcUnscrolledPosition(client_pos.x, client_pos.y, &x, &y);
        if (x > 120) {
            int clicked_track = hit_test_track(y);
            if (clicked_track >= 0)
                m_selected_track = clicked_track;
        }
    }

    wxMenu menu;
    menu.Append(ID_MENU_CUT, "Cut\tX", "Cut selection to clipboard");
    menu.Append(ID_MENU_COPY, "Copy\tC", "Copy selection to clipboard");
    menu.Append(ID_MENU_PASTE, "Paste\tV", "Paste from clipboard");
    menu.AppendSeparator();
    menu.Append(ID_MENU_SILENCE, "Silence\tS", "Silence selection");
    menu.Append(ID_MENU_INSERT_SILENCE, "Insert Gap\tI", "Insert gap at cursor");
    menu.AppendSeparator();
    menu.Append(ID_MENU_UNDO, "Undo\tCtrl+Z", "Undo last operation");
    menu.Append(ID_MENU_REDO, "Redo\tCtrl+Y", "Redo last operation");
    menu.AppendSeparator();
    menu.Append(ID_MENU_BEAT_QUANTIZE, "Beat Quantize...", "Quantize audio to beat grid");
    wxMenu* convert_menu = new wxMenu;
    convert_menu->Append(ID_MENU_CONVERT_SOUNDFONT, "SoundFont");
    convert_menu->Append(ID_MENU_CONVERT_SFZ, "SFZ");
    convert_menu->Append(ID_MENU_CONVERT_PLUGIN, "Plugin");
    menu.AppendSubMenu(convert_menu, "Convert to Notation Track");

    bool can_convert = can_convert_selected_track();
    menu.Enable(ID_MENU_CONVERT_SOUNDFONT, can_convert);
    menu.Enable(ID_MENU_CONVERT_SFZ, can_convert);
    menu.Enable(ID_MENU_CONVERT_PLUGIN, can_convert);
    
    // Bind menu events to operation handlers
    Bind(wxEVT_MENU, [this](wxCommandEvent& e) { do_cut(); }, ID_MENU_CUT);
    Bind(wxEVT_MENU, [this](wxCommandEvent& e) { do_copy(); }, ID_MENU_COPY);
    Bind(wxEVT_MENU, [this](wxCommandEvent& e) { do_paste(); }, ID_MENU_PASTE);
    Bind(wxEVT_MENU, [this](wxCommandEvent& e) { do_silence(); }, ID_MENU_SILENCE);
    Bind(wxEVT_MENU, [this](wxCommandEvent& e) { do_insert_silence(); }, ID_MENU_INSERT_SILENCE);
    Bind(wxEVT_MENU, [this](wxCommandEvent& e) { do_undo(); }, ID_MENU_UNDO);
    Bind(wxEVT_MENU, [this](wxCommandEvent& e) { do_redo(); }, ID_MENU_REDO);
    Bind(wxEVT_MENU, [this](wxCommandEvent& e) { do_beat_quantize(); }, ID_MENU_BEAT_QUANTIZE);
    menu.Bind(wxEVT_MENU, [this](wxCommandEvent&) { do_convert_to_notation_track(InstrumentType::SoundFont); }, ID_MENU_CONVERT_SOUNDFONT);
    menu.Bind(wxEVT_MENU, [this](wxCommandEvent&) { do_convert_to_notation_track(InstrumentType::SFZ); }, ID_MENU_CONVERT_SFZ);
    menu.Bind(wxEVT_MENU, [this](wxCommandEvent&) { do_convert_to_notation_track(InstrumentType::Plugin); }, ID_MENU_CONVERT_PLUGIN);
    
    if (event.GetPosition() == wxDefaultPosition) {
        PopupMenu(&menu);
    } else {
        PopupMenu(&menu, ScreenToClient(event.GetPosition()));
    }
}

void TracksView::OnKeyDown(wxKeyEvent& event) {
    int key = event.GetKeyCode();
    
    if (event.ControlDown() || event.CmdDown()) {
        if (key == 'Z' || key == 'z') {
            do_undo();
            return;
        } else if (key == 'Y' || key == 'y') {
            do_redo();
            return;
        }
    }
    
    switch (key) {
        case 'X':
        case 'x':
            do_cut();
            break;
        case 'C':
        case 'c':
            do_copy();
            break;
        case 'V':
        case 'v':
            do_paste();
            break;
        case 'S':
        case 's':
            do_silence();
            break;
        case 'I':
        case 'i':
            do_insert_silence();
            break;
        default:
            event.Skip();
            return;
    }
}

void TracksView::do_cut() {
    if (m_sel_start_tick == -1 || m_sel_end_tick == -1 || m_selected_track < 0) {
        return;
    }
    
    int num_tracks = (int)m_engine.track_count();
    if (m_selected_track >= num_tracks) return;
    
    auto& track = m_engine.track(m_selected_track);
    auto inst = track.instrument();
    if (!inst || inst->type() != InstrumentType::Sampler) {
        return;
    }
    
    auto sampler = static_cast<SampleInstrument*>(inst);
    
    int start_row = std::min(m_sel_start_tick, m_sel_end_tick);
    int end_row = std::max(m_sel_start_tick, m_sel_end_tick);
    if (start_row == end_row) end_row = start_row + 1;
    
    uint32_t lpb = m_engine.lpb();
    double bpm = m_engine.tempo();
    double sample_rate = m_engine.sample_rate();
    double samples_per_beat = (sample_rate * 60.0) / bpm;
    double samples_per_row = (lpb > 0) ? (samples_per_beat / lpb) : 44100.0;
    
    size_t start_sample_global = (size_t)(start_row * samples_per_row);
    size_t end_sample_global = (size_t)(end_row * samples_per_row);
    
    auto order = m_engine.order_list();
    size_t current_sample_pos = 0;
    
    // Map of sample_index to its modifications (start, end)
    struct Mod { size_t start; size_t end; };
    std::map<size_t, std::vector<Mod>> mods;
    
    for (size_t pat_idx = 0; pat_idx < order.size(); ++pat_idx) {
        auto& pattern = m_engine.pattern(order[pat_idx]);
        size_t pat_rows = pattern.row_count();
        size_t pat_samples = (size_t)(pat_rows * samples_per_row);
        
        for (size_t row = 0; row < pat_rows; ++row) {
            size_t row_sample_pos = current_sample_pos + (size_t)(row * samples_per_row);
            
            size_t num_cols = pattern.column_count(m_selected_track);
            for (size_t c = 0; c < num_cols; ++c) {
                auto& event = pattern.event(m_selected_track, row, c);
                if (event.note != 255 && event.note != 254) {
                    size_t s_idx = (event.sample_idx > 0) ? (event.sample_idx - 1) : sampler->selected_sample();
                    
                    if (s_idx < sampler->sample_count()) {
                        auto& sample_entry = sampler->get_sample(s_idx);
                        if (sample_entry.data) {
                            size_t sample_len = sample_entry.data->left.size();
                            size_t note_start_global = row_sample_pos;
                            size_t note_end_global = row_sample_pos + sample_len;
                            
                            size_t intersect_start = std::max(start_sample_global, note_start_global);
                            size_t intersect_end = std::min(end_sample_global, note_end_global);
                            
                            if (intersect_start < intersect_end) {
                                mods[s_idx].push_back({intersect_start - note_start_global, intersect_end - note_start_global});
                            }
                        }
                    }
                }
            }
        }
        current_sample_pos += pat_samples;
    }
    
    bool changed = false;
    for (auto& pair : mods) {
        size_t s_idx = pair.first;
        auto& sample_entry = sampler->get_sample(s_idx);
        
        // Create a working copy
        auto new_data = std::make_shared<SampleData>(*sample_entry.data);
        
        // Sort modifications in reverse order to keep indices valid while erasing
        std::sort(pair.second.begin(), pair.second.end(), [](const Mod& a, const Mod& b) {
            return a.start > b.start;
        });
        
        sampler->push_undo(s_idx);
        for (const auto& m : pair.second) {
            new_data->cut(m.start, m.end);
        }
        sampler->update_sample_data(s_idx, new_data);
        changed = true;
    }
    
    if (changed) {
        update_view();
        Refresh();
    }
}

void TracksView::do_copy() {
    if (m_sel_start_tick == -1 || m_sel_end_tick == -1 || m_selected_track < 0) {
        return;
    }

    int num_tracks = (int)m_engine.track_count();
    if (m_selected_track >= num_tracks) return;

    auto& track = m_engine.track(m_selected_track);
    auto inst = track.instrument();
    if (!inst || inst->type() != InstrumentType::Sampler) {
        return;
    }

    auto sampler = static_cast<SampleInstrument*>(inst);

    int start_row = std::min(m_sel_start_tick, m_sel_end_tick);
    int end_row = std::max(m_sel_start_tick, m_sel_end_tick);
    if (start_row == end_row) end_row = start_row + 1;

    uint32_t lpb = m_engine.lpb();
    double bpm = m_engine.tempo();
    double sample_rate = m_engine.sample_rate();
    double samples_per_beat = (sample_rate * 60.0) / bpm;
    double samples_per_row = (lpb > 0) ? (samples_per_beat / lpb) : 44100.0;

    size_t start_sample_global = (size_t)(start_row * samples_per_row);
    size_t end_sample_global = (size_t)(end_row * samples_per_row);

    auto order = m_engine.order_list();
    size_t current_sample_pos = 0;

    for (size_t pat_idx = 0; pat_idx < order.size(); ++pat_idx) {
        auto& pattern = m_engine.pattern(order[pat_idx]);
        size_t pat_rows = pattern.row_count();
        size_t pat_samples = (size_t)(pat_rows * samples_per_row);

        for (size_t row = 0; row < pat_rows; ++row) {
            size_t row_sample_pos = current_sample_pos + (size_t)(row * samples_per_row);
            
            size_t num_cols = pattern.column_count(m_selected_track);
            for (size_t c = 0; c < num_cols; ++c) {
                auto& event = pattern.event(m_selected_track, row, c);
                if (event.note != 255 && event.note != 254) {
                    size_t s_idx = (event.sample_idx > 0) ? (event.sample_idx - 1) : sampler->selected_sample();
                    if (s_idx < sampler->sample_count()) {
                        auto& sample_entry = sampler->get_sample(s_idx);
                        if (sample_entry.data) {
                            size_t sample_len = sample_entry.data->left.size();
                            size_t note_start_global = row_sample_pos;
                            size_t note_end_global = row_sample_pos + sample_len;

                            size_t intersect_start = std::max(start_sample_global, note_start_global);
                            size_t intersect_end = std::min(end_sample_global, note_end_global);

                            if (intersect_start < intersect_end) {
                                // Just copy the first one we find for now
                                auto cmd = std::make_unique<TrackCopyCommand>(track, m_engine, intersect_start - note_start_global, intersect_end - note_start_global);
                                cmd->apply();
                                return;
                            }
                        }
                    }
                }
            }
        }
        current_sample_pos += pat_samples;
    }
}

void TracksView::do_paste() {
    if (m_sel_start_tick == -1 || m_selected_track < 0) {
        return;
    }

    int num_tracks = (int)m_engine.track_count();
    if (m_selected_track >= num_tracks) return;

    auto& track = m_engine.track(m_selected_track);
    auto inst = track.instrument();
    if (!inst || inst->type() != InstrumentType::Sampler) {
        return;
    }

    auto sampler = static_cast<SampleInstrument*>(inst);

    int cursor_row = m_sel_start_tick;

    uint32_t lpb = m_engine.lpb();
    double bpm = m_engine.tempo();
    double sample_rate = m_engine.sample_rate();
    double samples_per_beat = (sample_rate * 60.0) / bpm;
    double samples_per_row = (lpb > 0) ? (samples_per_beat / lpb) : 44100.0;

    size_t paste_pos_global = (size_t)(cursor_row * samples_per_row);

    auto order = m_engine.order_list();
    size_t current_sample_pos = 0;

    for (size_t pat_idx = 0; pat_idx < order.size(); ++pat_idx) {
        auto& pattern = m_engine.pattern(order[pat_idx]);
        size_t pat_rows = pattern.row_count();
        size_t pat_samples = (size_t)(pat_rows * samples_per_row);

        for (size_t row = 0; row < pat_rows; ++row) {
            size_t row_sample_pos = current_sample_pos + (size_t)(row * samples_per_row);
            
            size_t num_cols = pattern.column_count(m_selected_track);
            for (size_t c = 0; c < num_cols; ++c) {
                auto& event = pattern.event(m_selected_track, row, c);
                if (event.note != 255 && event.note != 254) {
                    size_t s_idx = (event.sample_idx > 0) ? (event.sample_idx - 1) : sampler->selected_sample();
                    if (s_idx < sampler->sample_count()) {
                        auto& sample_entry = sampler->get_sample(s_idx);
                        if (sample_entry.data) {
                            size_t sample_len = sample_entry.data->left.size();
                            if (paste_pos_global >= row_sample_pos && 
                                paste_pos_global < row_sample_pos + sample_len) {
                                auto cmd = std::make_unique<TrackPasteCommand>(track, m_engine, paste_pos_global - row_sample_pos);
                                cmd->apply();
                                update_view();
                                Refresh();
                                return;
                            }
                        }
                    }
                }
            }
        }
        current_sample_pos += pat_samples;
    }
}

void TracksView::do_silence() {
    if (m_sel_start_tick == -1 || m_sel_end_tick == -1 || m_selected_track < 0) {
        return;
    }
    
    int num_tracks = (int)m_engine.track_count();
    if (m_selected_track >= num_tracks) return;
    
    auto& track = m_engine.track(m_selected_track);
    auto inst = track.instrument();
    if (!inst || inst->type() != InstrumentType::Sampler) {
        return;
    }
    
    auto sampler = static_cast<SampleInstrument*>(inst);
    
    int start_row = std::min(m_sel_start_tick, m_sel_end_tick);
    int end_row = std::max(m_sel_start_tick, m_sel_end_tick);
    if (start_row == end_row) end_row = start_row + 1;
    
    uint32_t lpb = m_engine.lpb();
    double bpm = m_engine.tempo();
    double sample_rate = m_engine.sample_rate();
    double samples_per_beat = (sample_rate * 60.0) / bpm;
    double samples_per_row = (lpb > 0) ? (samples_per_beat / lpb) : 44100.0;
    
    size_t start_sample_global = (size_t)(start_row * samples_per_row);
    size_t end_sample_global = (size_t)(end_row * samples_per_row);
    
    auto order = m_engine.order_list();
    size_t current_sample_pos = 0;
    
    struct Mod { size_t start; size_t end; };
    std::map<size_t, std::vector<Mod>> mods;
    
    for (size_t pat_idx = 0; pat_idx < order.size(); ++pat_idx) {
        auto& pattern = m_engine.pattern(order[pat_idx]);
        size_t pat_rows = pattern.row_count();
        size_t pat_samples = (size_t)(pat_rows * samples_per_row);
        
        for (size_t row = 0; row < pat_rows; ++row) {
            size_t row_sample_pos = current_sample_pos + (size_t)(row * samples_per_row);
            
            size_t num_cols = pattern.column_count(m_selected_track);
            for (size_t c = 0; c < num_cols; ++c) {
                auto& event = pattern.event(m_selected_track, row, c);
                if (event.note != 255 && event.note != 254) {
                    size_t s_idx = (event.sample_idx > 0) ? (event.sample_idx - 1) : sampler->selected_sample();
                    
                    if (s_idx < sampler->sample_count()) {
                        auto& sample_entry = sampler->get_sample(s_idx);
                        if (sample_entry.data) {
                            size_t sample_len = sample_entry.data->left.size();
                            size_t note_start_global = row_sample_pos;
                            size_t note_end_global = row_sample_pos + sample_len;
                            
                            size_t intersect_start = std::max(start_sample_global, note_start_global);
                            size_t intersect_end = std::min(end_sample_global, note_end_global);
                            
                            if (intersect_start < intersect_end) {
                                mods[s_idx].push_back({intersect_start - note_start_global, intersect_end - note_start_global});
                            }
                        }
                    }
                }
            }
        }
        current_sample_pos += pat_samples;
    }
    
    bool changed = false;
    for (auto& pair : mods) {
        size_t s_idx = pair.first;
        auto& sample_entry = sampler->get_sample(s_idx);
        auto new_data = std::make_shared<SampleData>(*sample_entry.data);
        
        sampler->push_undo(s_idx);
        for (const auto& m : pair.second) {
            new_data->silence(m.start, m.end);
        }
        sampler->update_sample_data(s_idx, new_data);
        changed = true;
    }
    
    if (changed) {
        Refresh();
    }
}

void TracksView::do_insert_silence() {
    if (m_sel_start_tick == -1 || m_selected_track < 0) {
        return;
    }
    
    int num_tracks = (int)m_engine.track_count();
    if (m_selected_track >= num_tracks) return;
    
    auto& track = m_engine.track(m_selected_track);
    auto inst = track.instrument();
    if (!inst || inst->type() != InstrumentType::Sampler) {
        return;
    }
    
    auto sampler = static_cast<SampleInstrument*>(inst);
    
    uint32_t lpb = m_engine.lpb();
    double bpm = m_engine.tempo();
    double sample_rate = m_engine.sample_rate();
    double samples_per_beat = (sample_rate * 60.0) / bpm;
    double samples_per_row = (lpb > 0) ? (samples_per_beat / lpb) : 44100.0;
    
    int cursor_row = m_sel_start_tick;
    size_t insert_pos_global = (size_t)(cursor_row * samples_per_row);
    
    size_t insert_duration = (size_t)samples_per_row;
    if (m_sel_end_tick != -1 && m_sel_end_tick != m_sel_start_tick) {
        int start_row = std::min(m_sel_start_tick, m_sel_end_tick);
        int end_row = std::max(m_sel_start_tick, m_sel_end_tick);
        insert_duration = (size_t)((end_row - start_row) * samples_per_row);
        insert_pos_global = (size_t)(start_row * samples_per_row);
    }
    
    auto order = m_engine.order_list();
    size_t current_sample_pos = 0;
    
    struct Mod { size_t pos; };
    std::map<size_t, std::vector<Mod>> mods;
    
    for (size_t pat_idx = 0; pat_idx < order.size(); ++pat_idx) {
        auto& pattern = m_engine.pattern(order[pat_idx]);
        size_t pat_rows = pattern.row_count();
        size_t pat_samples = (size_t)(pat_rows * samples_per_row);
        
        for (size_t row = 0; row < pat_rows; ++row) {
            size_t row_sample_pos = current_sample_pos + (size_t)(row * samples_per_row);
            
            size_t num_cols = pattern.column_count(m_selected_track);
            for (size_t c = 0; c < num_cols; ++c) {
                auto& event = pattern.event(m_selected_track, row, c);
                if (event.note != 255 && event.note != 254) {
                    size_t s_idx = (event.sample_idx > 0) ? (event.sample_idx - 1) : sampler->selected_sample();
                    
                    if (s_idx < sampler->sample_count()) {
                        auto& sample_entry = sampler->get_sample(s_idx);
                        if (sample_entry.data) {
                            size_t sample_len = sample_entry.data->left.size();
                            if (insert_pos_global >= row_sample_pos && 
                                insert_pos_global < row_sample_pos + sample_len) {
                                mods[s_idx].push_back({insert_pos_global - row_sample_pos});
                            }
                        }
                    }
                }
            }
        }
        current_sample_pos += pat_samples;
    }
    
    bool changed = false;
    for (auto& pair : mods) {
        size_t s_idx = pair.first;
        auto& sample_entry = sampler->get_sample(s_idx);
        auto new_data = std::make_shared<SampleData>(*sample_entry.data);
        
        // Sort in reverse
        std::sort(pair.second.begin(), pair.second.end(), [](const Mod& a, const Mod& b) {
            return a.pos > b.pos;
        });
        
        sampler->push_undo(s_idx);
        for (const auto& m : pair.second) {
            new_data->insert_silence(m.pos, insert_duration);
        }
        sampler->update_sample_data(s_idx, new_data);
        changed = true;
    }
    
    if (changed) {
        update_view();
        Refresh();
    }
}

void TracksView::do_undo() {
    m_engine.undo_stack().undo();
    Refresh();
}

void TracksView::do_redo() {
    m_engine.undo_stack().redo();
    Refresh();
}

void TracksView::do_beat_quantize() {
    if (m_selected_track < 0 || m_sel_start_tick < 0 || m_sel_end_tick < 0) {
        wxMessageBox("Please select a region on a sampler track first.",
                     "Beat Quantize", wxOK | wxICON_INFORMATION);
        return;
    }

    auto& track_obj = m_engine.track(m_selected_track);
    auto* inst = track_obj.instrument();
    auto* sampler = dynamic_cast<SampleInstrument*>(inst);
    if (!sampler || sampler->sample_count() == 0) {
        wxMessageBox("Beat Quantize only works on Sampler tracks with loaded samples.",
                     "Beat Quantize", wxOK | wxICON_WARNING);
        return;
    }

    size_t sample_idx = sampler->selected_sample();
    auto src_data = sampler->get_sample(sample_idx).data;
    if (!src_data || src_data->left.empty()) {
        wxMessageBox("No audio data in the selected sample slot.",
                     "Beat Quantize", wxOK | wxICON_WARNING);
        return;
    }

    double bpm = m_engine.tempo();
    int lpb    = (int)m_engine.lpb();
    int sr     = (int)m_engine.sample_rate();
    double samples_per_row = (sr * 60.0) / (bpm * lpb);

    int t1 = std::min(m_sel_start_tick, m_sel_end_tick);
    int t2 = std::max(m_sel_start_tick, m_sel_end_tick);
    size_t s_start = (size_t)(t1 * samples_per_row);
    size_t s_end   = (size_t)(t2 * samples_per_row);
    s_start = std::min(s_start, src_data->left.size());
    s_end   = std::min(s_end,   src_data->left.size());
    if (s_start >= s_end) {
        wxMessageBox("Selection is empty or too small.",
                     "Beat Quantize", wxOK | wxICON_INFORMATION);
        return;
    }

    // Extract selected region
    auto region = std::make_shared<SampleData>();
    region->sample_rate = src_data->sample_rate;
    region->left.assign(src_data->left.begin() + s_start, src_data->left.begin() + s_end);
    if (!src_data->right.empty())
        region->right.assign(src_data->right.begin() + s_start, src_data->right.begin() + s_end);

    BeatQuantizeDialog dlg(this, m_engine, region, m_selected_track);
    if (dlg.ShowModal() != wxID_OK) return;

    auto result = dlg.get_result();
    if (!result || result->left.empty()) return;

    size_t new_len = result->left.size();

    sampler->push_undo(sample_idx);

    src_data->left.erase(src_data->left.begin() + s_start, src_data->left.begin() + s_end);
    src_data->left.insert(src_data->left.begin() + s_start, result->left.begin(), result->left.end());
    if (!src_data->right.empty() && !result->right.empty()) {
        src_data->right.erase(src_data->right.begin() + s_start, src_data->right.begin() + s_end);
        src_data->right.insert(src_data->right.begin() + s_start, result->right.begin(), result->right.end());
    }

    update_view();
    Refresh();
    wxMessageBox(
        wxString::Format("Beat quantize applied: %zu → %zu samples.",
                         s_end - s_start, new_len),
        "Beat Quantize", wxOK | wxICON_INFORMATION);
}

void TracksView::do_convert_to_notation_track(InstrumentType dest_type) {
    if (!can_convert_selected_track()) {
        wxMessageBox("Choose a sampler track with audio before converting.",
                     "Convert to Notation Track", wxOK | wxICON_INFORMATION);
        return;
    }

    Engine::TrackConversionOptions opts;
    opts.notation = NotationType::Violin;

    size_t new_track = 0;
    std::string error;
    if (!m_engine.convert_sampler_track_to_notation_track((size_t)m_selected_track, dest_type, opts, &new_track, &error)) {
        wxMessageBox(error.empty() ? "Track conversion failed." : wxString::FromUTF8(error),
                     "Convert to Notation Track", wxOK | wxICON_WARNING);
        return;
    }

    m_selected_track = (int)new_track;
    update_view();
    Refresh();

    if (auto* main_window = dynamic_cast<WxMainWindow*>(wxGetTopLevelParent(this))) {
        main_window->update_all_uis();
    }

    const char* type_name = (dest_type == InstrumentType::SoundFont) ? "SoundFont" :
                            (dest_type == InstrumentType::SFZ) ? "SFZ" : "Plugin";
    wxMessageBox(wxString::Format("Created a new %s notation track from the full sampler track.", type_name),
                 "Convert to Notation Track", wxOK | wxICON_INFORMATION);
}

std::vector<TracksView::AudioRegion> TracksView::detect_overlaps(const SampleInstrument* sampler) {
    std::vector<AudioRegion> regions;
    
    if (!sampler || sampler->sample_count() == 0) {
        return regions;
    }
    
    for (size_t i = 0; i < sampler->sample_count(); ++i) {
        auto& sample = sampler->get_sample(i);
        if (sample.data && !sample.data->left.empty()) {
            AudioRegion region;
            region.start_sample = 0;
            region.end_sample = sample.data->left.size();
            region.is_overlapping = false;
            regions.push_back(region);
        }
    }
    
    for (size_t i = 0; i < regions.size(); ++i) {
        for (size_t j = i + 1; j < regions.size(); ++j) {
            if (regions[i].start_sample < regions[j].end_sample &&
                regions[j].start_sample < regions[i].end_sample) {
                regions[i].is_overlapping = true;
                regions[j].is_overlapping = true;
            }
        }
    }
    
    return regions;
}

} // namespace disgrace_ns
