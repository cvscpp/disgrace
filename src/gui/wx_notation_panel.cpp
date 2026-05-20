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
#include <wx/msgdlg.h>
#include <wx/utils.h>
#include "wx_notation_panel.h"
#include "wx_detached_frame.h"
#include "../core/engine.h"
#include "../io/lilypond_exporter.h"
#include "theme.h"
#include <fstream>

namespace disgrace_ns {

enum {
    ID_ZOOM_IN = 10101,
    ID_ZOOM_OUT,
    ID_VIEW_ALL,
    ID_VIEW_SEL,
    ID_PREVIEW_ALL,
    ID_DETACH,
    ID_PREVIEW_BASE = 10200
};

// ---------------------------------------------------------------------------
// Notation rendering constants
// ---------------------------------------------------------------------------
static constexpr int N_LINE_SP  = 10;   // pixels between staff lines
static constexpr int N_TRACK_H  = 160;  // per-track height in pixels
static constexpr int N_HEADER_W = 120;  // left header column width
static constexpr int N_STAFF_Y  = 35;   // offset from track top to first staff line
static constexpr int N_CLEF_W   = 62;   // pixels reserved for clef + time sig

// ---------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(NotationPanel, wxPanel)
    EVT_BUTTON(ID_ZOOM_IN, NotationPanel::on_zoom_in)
    EVT_BUTTON(ID_ZOOM_OUT, NotationPanel::on_zoom_out)
    EVT_BUTTON(ID_VIEW_ALL, NotationPanel::on_view_all)
    EVT_BUTTON(ID_VIEW_SEL, NotationPanel::on_view_sel)
    EVT_BUTTON(ID_PREVIEW_ALL, NotationPanel::on_preview_all)
    EVT_BUTTON(ID_DETACH, NotationPanel::on_detach)
wxEND_EVENT_TABLE()

NotationPanel::NotationPanel(wxWindow* parent, Engine& engine)
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
    m_preview_all_btn = new wxButton(this, ID_PREVIEW_ALL, "Preview All", wxDefaultPosition, wxSize(-1, btn_h));
    m_preview_all_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_BUTTON, wxSize(16, 16)));
    m_detach_btn = new wxButton(this, ID_DETACH, "", wxDefaultPosition, wxSize(btn_h, btn_h));
    m_detach_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_FULL_SCREEN, wxART_BUTTON, wxSize(16, 16)));
    m_detach_btn->SetToolTip("Detach");

    btn_sizer->Add(m_zoom_in_btn, 0, wxALL, 2);
    btn_sizer->Add(m_zoom_out_btn, 0, wxALL, 2);
    btn_sizer->Add(m_view_all_btn, 0, wxALL, 2);
    btn_sizer->Add(m_view_sel_btn, 0, wxALL, 2);
    btn_sizer->Add(m_preview_all_btn, 0, wxALL, 2);
    btn_sizer->Add(m_detach_btn, 0, wxALL, 2);

    main_sizer->Add(btn_sizer, 0, wxEXPAND | wxALL, 2);

    m_notation_view = new NotationView(this, wxID_ANY, m_engine);
    main_sizer->Add(m_notation_view, 1, wxEXPAND | wxALL, 0);

    SetSizer(main_sizer);
}

void NotationPanel::update() {
    if (m_engine.track_count() != m_last_track_count) {
        m_last_track_count = m_engine.track_count();
        m_notation_view->update_view();
    }
    m_notation_view->Refresh();
}

void NotationPanel::on_zoom_in(wxCommandEvent& event) { m_notation_view->zoom_in(); }
void NotationPanel::on_zoom_out(wxCommandEvent& event) { m_notation_view->zoom_out(); }
void NotationPanel::on_view_all(wxCommandEvent& event) { m_notation_view->view_all(); }
void NotationPanel::on_view_sel(wxCommandEvent& event) { m_notation_view->view_selection(); }
void NotationPanel::on_preview_all(wxCommandEvent& event) {
    std::string ly_src = LilyPondExporter::generate_ly_source(m_engine, -1);
    
    std::string tmp_ly = "/tmp/disgrace_preview_all.ly";
    std::string tmp_pdf = "/tmp/disgrace_preview_all.pdf";
    std::ofstream f(tmp_ly);
    if (f.is_open()) {
        f << ly_src;
        f.close();
        
        std::string cmd = "lilypond -o /tmp/disgrace_preview_all " + tmp_ly;
        if (wxExecute(cmd, wxEXEC_SYNC) == 0) {
            wxLaunchDefaultApplication(tmp_pdf);
        } else {
            wxMessageBox("LilyPond execution failed. Make sure 'lilypond' is installed and in your PATH.", "Error", wxOK | wxICON_ERROR);
        }
    }
}
void NotationPanel::on_detach(wxCommandEvent& event) {
    if (m_detached_frame) {
        return;
    }
    Hide();
    m_detached_frame = new DetachedFrame(this, "Notation", GetParent(), m_tab_index);
    m_detached_frame->set_on_detach_callback([this]() { m_detached_frame = nullptr; });
}

wxBEGIN_EVENT_TABLE(NotationView, wxScrolledWindow)
    EVT_PAINT(NotationView::OnPaint)
    EVT_SIZE(NotationView::OnSize)
    EVT_LEFT_DOWN(NotationView::OnMouseDown)
    EVT_MOTION(NotationView::OnMouseDrag)
    EVT_LEFT_UP(NotationView::OnMouseUp)
    EVT_MOUSEWHEEL(NotationView::OnMouseWheel)
wxEND_EVENT_TABLE()

NotationView::NotationView(wxWindow* parent, wxWindowID id, Engine& engine)
    : wxScrolledWindow(parent, id), m_engine(engine), m_zoom(25.0) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetScrollRate(1, 1);
    m_needs_initial_view_all = true;
}

void NotationView::OnSize(wxSizeEvent& event) {
    if (m_needs_initial_view_all && GetClientSize().x > 150) {
        view_all();
    }
    event.Skip();
}

void NotationView::zoom_in() { m_zoom *= 1.5; update_view(); }
void NotationView::zoom_out() { m_zoom /= 1.5; if (m_zoom < 1.0) m_zoom = 1.0; update_view(); }
void NotationView::view_all() {
    int total = get_total_ticks();
    if (total > 0) {
        wxSize size = GetClientSize();
        if (size.GetWidth() <= 150) size = GetParent()->GetClientSize();
        if (size.GetWidth() > 150) {
            m_zoom = (double)(size.GetWidth() - 140) / total;
            if (m_zoom < 1.0) m_zoom = 1.0;
            m_needs_initial_view_all = false;
        }
    }
    update_view();
}
void NotationView::view_selection() {
    if (m_sel_start_tick != -1 && m_sel_end_tick != -1) {
        int diff = std::abs(m_sel_end_tick - m_sel_start_tick);
        if (diff > 0) {
            wxSize size = GetClientSize();
            if (size.GetWidth() <= 0) size = GetParent()->GetClientSize();
            m_zoom = (double)(size.GetWidth() - 140) / diff;
            if (m_zoom < 1.0) m_zoom = 1.0;
        }
    }
    update_view();
}

void NotationView::update_view() {
    int total_w = N_HEADER_W + N_CLEF_W + tick_to_x(get_total_ticks()) + 60;
    int total_h = 30 + (int)m_engine.track_count() * N_TRACK_H + 50;
    SetVirtualSize(total_w, total_h);

    for (auto* btn : m_preview_buttons) btn->Destroy();
    m_preview_buttons.clear();

    for (int t = 0; t < (int)m_engine.track_count(); ++t) {
        auto& track_obj = m_engine.track(t);
        Instrument* inst = track_obj.instrument();
        if (inst && (inst->type() == InstrumentType::SoundFont ||
                     inst->type() == InstrumentType::SFZ ||
                     inst->type() == InstrumentType::Plugin ||
                     inst->type() == InstrumentType::Midi)) {
            int ty = 30 + t * N_TRACK_H;
            wxButton* b = new wxButton(this, ID_PREVIEW_BASE + t, "Preview (LY)",
                                       wxPoint(5, ty + N_TRACK_H - 32), wxSize(110, 25));
            b->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_BUTTON, wxSize(14, 14)));
            wxFont btn_font(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
            b->SetFont(btn_font);
            b->Bind(wxEVT_BUTTON, &NotationView::on_preview_track, this);
            m_preview_buttons.push_back(b);
        }
    }
    Refresh();
}

void NotationView::on_preview_track(wxCommandEvent& event) {
    int t_idx = event.GetId() - ID_PREVIEW_BASE;
    if (t_idx < 0 || t_idx >= (int)m_engine.track_count()) return;

    std::string ly_src = LilyPondExporter::generate_ly_source(m_engine, t_idx);
    
    std::string tmp_ly = "/tmp/disgrace_preview.ly";
    std::string tmp_pdf = "/tmp/disgrace_preview.pdf";
    std::ofstream f(tmp_ly);
    if (f.is_open()) {
        f << ly_src;
        f.close();
        
        std::string cmd = "lilypond -o /tmp/disgrace_preview " + tmp_ly;
        if (wxExecute(cmd, wxEXEC_SYNC) == 0) {
            wxLaunchDefaultApplication(tmp_pdf);
        } else {
            wxMessageBox("LilyPond execution failed. Make sure 'lilypond' is installed and in your PATH.", "Error", wxOK | wxICON_ERROR);
        }
    }
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

void NotationView::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    PrepareDC(dc);
    draw(dc);
}

// ---------------------------------------------------------------------------
// Clef drawing helpers
// ---------------------------------------------------------------------------

// Draw treble clef glyph (𝄞) centred so the G-line (staff_top + 3×N_LINE_SP)
// is the "curl" point of the clef.  Falls back to a polygon sketch when the
// musical font is not available.
void NotationView::draw_clef_treble(wxDC& dc, int x, int staff_top) {
    wxColour ink = ThemeManager::toWxColour(m_engine.m_tracker_text);
    dc.SetTextForeground(ink);
    dc.SetPen(wxPen(ink));
    dc.SetBrush(wxBrush(ink));

    // Try Unicode musical symbol via FreeSerif (common on Linux, has U+1D11E)
    wxFont mf(4 * N_LINE_SP, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "FreeSerif");
    dc.SetFont(mf);
    wxString clef_ch = wxString::FromUTF8("\xF0\x9D\x84\x9E");  // U+1D11E 𝄞
    wxSize ext = dc.GetTextExtent(clef_ch);
    if (ext.GetWidth() > 4) {
        // G line is staff_top + 3×N_LINE_SP.  Position so ~68% of glyph height
        // aligns with the G line.
        int glyph_y = (staff_top + 3 * N_LINE_SP) - (int)(ext.GetHeight() * 0.68);
        dc.DrawText(clef_ch, x + 2, glyph_y);
        return;
    }

    // Fallback polygon sketch
    int cx = x + 12;
    int g_line = staff_top + 3 * N_LINE_SP;  // G line — curl of the treble clef
    int hs = N_LINE_SP;                        // half-space unit

    // Vertical stem from 3 spaces above top line to 3 spaces below bottom line
    dc.DrawLine(cx, staff_top - 3*hs, cx, staff_top + 4*N_LINE_SP + 3*hs);

    // Oval body around B4 line (space between 3rd and 4th lines from top)
    int body_cy = staff_top + 3*N_LINE_SP + hs/2;
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawEllipse(cx - hs, body_cy - hs, 2*hs, (int)(1.4*hs));

    // Upper loop centred on G line
    dc.DrawEllipse(cx - hs - 2, g_line - hs, 2*hs + 2, 2*hs);

    // Small curl at bottom of stem
    dc.DrawEllipse(cx - hs/2, staff_top + 4*N_LINE_SP + 2*hs, hs, hs);
}

// Draw bass clef glyph (𝄢) so the F-line (staff_top + N_LINE_SP) passes
// through the right notch of the "C" shape.
void NotationView::draw_clef_bass(wxDC& dc, int x, int staff_top) {
    wxColour ink = ThemeManager::toWxColour(m_engine.m_tracker_text);
    dc.SetTextForeground(ink);
    dc.SetPen(wxPen(ink));
    dc.SetBrush(wxBrush(ink));

    wxFont mf(3 * N_LINE_SP, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "FreeSerif");
    dc.SetFont(mf);
    wxString clef_ch = wxString::FromUTF8("\xF0\x9D\x84\xA2");  // U+1D122 𝄢
    wxSize ext = dc.GetTextExtent(clef_ch);
    if (ext.GetWidth() > 4) {
        // F line is staff_top + N_LINE_SP.  Align ~28% of glyph height there.
        int glyph_y = (staff_top + N_LINE_SP) - (int)(ext.GetHeight() * 0.28);
        dc.DrawText(clef_ch, x + 2, glyph_y);
        return;
    }

    // Fallback sketch
    int cx = x + 8;
    int f_line = staff_top + N_LINE_SP;  // F line (2nd from top)
    int hs = N_LINE_SP;

    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    // "C" arc body centred between F line and 3rd staff line
    int body_cy = f_line + hs + hs/2;
    dc.DrawEllipse(cx - hs, body_cy - (int)(1.5*hs), 2*hs, (int)(3.0*hs));
    // Cover right half to make "C"
    dc.SetBrush(wxBrush(ThemeManager::toWxColour(m_engine.m_tracker_bg)));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(cx, body_cy - 2*hs, hs + 4, 4*hs);
    // Two dots to the right of the notch
    dc.SetBrush(wxBrush(ink));
    dc.SetPen(wxPen(ink));
    dc.DrawCircle(cx + hs + 4, f_line,      3);
    dc.DrawCircle(cx + hs + 4, f_line + hs, 3);
}

// Draw percussion clef (two vertical bars).
void NotationView::draw_clef_perc(wxDC& dc, int x, int staff_top) {
    wxColour ink = ThemeManager::toWxColour(m_engine.m_tracker_text);
    dc.SetPen(wxPen(ink, 3));
    dc.SetBrush(wxBrush(ink));
    int top = staff_top;
    int bot = staff_top + 4 * N_LINE_SP;
    dc.DrawLine(x + 6,  top, x + 6,  bot);
    dc.DrawLine(x + 12, top, x + 12, bot);
}

// Draw "n/d" time signature stacked numerals.
void NotationView::draw_time_sig(wxDC& dc, int x, int staff_top, int n, int d) {
    wxColour ink = ThemeManager::toWxColour(m_engine.m_tracker_text);
    dc.SetTextForeground(ink);
    wxFont tsf(2 * N_LINE_SP, wxFONTFAMILY_ROMAN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
    dc.SetFont(tsf);
    wxString top_s = wxString::Format("%d", n);
    wxString bot_s = wxString::Format("%d", d);
    wxSize ext = dc.GetTextExtent(top_s);
    int mid = staff_top + 2 * N_LINE_SP;  // middle staff line
    dc.DrawText(top_s, x, mid - ext.GetHeight());
    dc.DrawText(bot_s, x, mid);
}

// ---------------------------------------------------------------------------
// Staff drawing
// ---------------------------------------------------------------------------

void NotationView::draw_staff(wxDC& dc, int tx, int ty, int tw, int type) {
    wxColour ink = ThemeManager::toWxColour(m_engine.m_tracker_text);
    dc.SetPen(wxPen(ink));

    auto draw_5_lines = [&](int staff_top) {
        for (int i = 0; i < 5; ++i)
            dc.DrawLine(tx, staff_top + i * N_LINE_SP,
                        tx + tw, staff_top + i * N_LINE_SP);
    };

    // Grand staff has two 5-line staves; others have one.
    int treble_top = ty + N_STAFF_Y;
    int bass_top   = ty + N_STAFF_Y + 6 * N_LINE_SP;  // gap of 2×N_LINE_SP between staves

    switch (type) {
        case 0:  // Treble
            draw_5_lines(treble_top);
            draw_clef_treble(dc, tx + 2, treble_top);
            draw_time_sig(dc, tx + 36, treble_top, 4, 4);
            break;
        case 1:  // Bass
            draw_5_lines(treble_top);
            draw_clef_bass(dc, tx + 2, treble_top);
            draw_time_sig(dc, tx + 36, treble_top, 4, 4);
            break;
        case 2:  // Grand staff (treble + bass)
            draw_5_lines(treble_top);
            draw_5_lines(bass_top);
            draw_clef_treble(dc, tx + 2, treble_top);
            draw_clef_bass(dc, tx + 2, bass_top);
            draw_time_sig(dc, tx + 36, treble_top, 4, 4);
            // Brace-bar connecting the two staves
            dc.DrawLine(tx, treble_top, tx, bass_top + 4 * N_LINE_SP);
            break;
        case 3:  // Percussion
            draw_5_lines(treble_top);
            draw_clef_perc(dc, tx + 4, treble_top);
            break;
        default:
            draw_5_lines(treble_top);
            break;
    }
}

// ---------------------------------------------------------------------------
// Ledger lines
// ---------------------------------------------------------------------------

void NotationView::draw_ledger_lines(wxDC& dc, int nx, int note_y, int staff_top) {
    // note_y is the vertical centre of the note head.
    // Draw short horizontal lines for every N_LINE_SP step above/below staff.
    int ledger_half = (int)(N_LINE_SP * 0.7);
    dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_text)));

    // Lines above staff (above staff_top)
    for (int ly = staff_top - N_LINE_SP; ly >= note_y - N_LINE_SP / 2; ly -= N_LINE_SP)
        dc.DrawLine(nx - ledger_half, ly, nx + ledger_half, ly);

    // Lines below staff (below bottom line = staff_top + 4×N_LINE_SP)
    int bot = staff_top + 4 * N_LINE_SP;
    for (int ly = bot + N_LINE_SP; ly <= note_y + N_LINE_SP / 2; ly += N_LINE_SP)
        dc.DrawLine(nx - ledger_half, ly, nx + ledger_half, ly);
}

// ---------------------------------------------------------------------------
// Note drawing
// ---------------------------------------------------------------------------

// pitch_to_pos[p]: diatonic step (0=C, 1=D, … 6=B) for semitone p in an octave.
static const int s_pitch_to_pos[12] = {0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6};
static const bool s_is_sharp[12]    = {false, true, false, true, false, false,
                                        true, false, true, false, true, false};

void NotationView::draw_note(wxDC& dc, int nx, int track_y, int note, int staff_type) {
    // Diatonic position relative to C4 (MIDI note 48 in this app's convention)
    int octave = note / 12;
    int pitch  = note % 12;
    int pos    = s_pitch_to_pos[pitch] + (octave - 4) * 7;  // steps above C4

    int staff_top = track_y + N_STAFF_Y;

    // dy: pixel offset from staff_top to the note centre.
    // Positive dy = downward (lower note).
    // Treble clef: pos=10 → E5 (top line), pos=0 → E4 (first space below staff).
    //   dy = (10 - pos) * half_step   → E5 at staff_top, B4 at 2nd line, etc.
    // Bass clef:   pos=12 → A3 (top line), pos=2 → A2 (first space below staff).
    //   dy = -(pos + 2) * half_step  (note: bass uses different reference)
    int hs = N_LINE_SP / 2;  // half-step = one diatonic slot
    int dy = 0;
    int treble_top = staff_top;
    int bass_top   = staff_top + 6 * N_LINE_SP;

    int used_staff_top = treble_top;

    if (staff_type == 0 || staff_type == 3) {  // treble / percussion
        dy = (10 - pos) * hs;
        used_staff_top = treble_top;
    } else if (staff_type == 1) {  // bass (fixed sign)
        dy = -(pos + 2) * hs;
        used_staff_top = treble_top;
    } else if (staff_type == 2) {  // grand staff — treble for upper, bass for lower
        if (pos >= 0) {  // C4 and above → treble
            dy = (10 - pos) * hs;
            used_staff_top = treble_top;
        } else {         // below C4 → bass
            dy = -(pos + 2) * hs;
            used_staff_top = bass_top;
        }
    }

    int note_y = used_staff_top + dy;  // vertical centre of note head

    // Note head dimensions (scaled with N_LINE_SP)
    int head_w = (int)(N_LINE_SP * 1.1);
    int head_h = (int)(N_LINE_SP * 0.75);

    // Ledger lines when outside the staff
    draw_ledger_lines(dc, nx, note_y, used_staff_top);

    // Note head
    wxColour note_col = ThemeManager::toWxColour(m_engine.m_tracker_note);
    dc.SetBrush(wxBrush(note_col));
    dc.SetPen(wxPen(note_col));
    dc.DrawEllipse(nx - head_w / 2, note_y - head_h / 2, head_w, head_h);

    // Stem: if note is in the lower half of staff → stem up on right side;
    //       if in upper half → stem down on left side.
    int staff_mid = used_staff_top + 2 * N_LINE_SP;
    int stem_len  = 3 * N_LINE_SP;
    dc.SetPen(wxPen(note_col));
    if (note_y >= staff_mid) {
        // Stem up
        dc.DrawLine(nx + head_w / 2 - 1, note_y,
                    nx + head_w / 2 - 1, note_y - stem_len);
    } else {
        // Stem down
        dc.DrawLine(nx - head_w / 2 + 1, note_y,
                    nx - head_w / 2 + 1, note_y + stem_len);
    }

    // Accidental (sharp ♯ or flat ♭)
    if (s_is_sharp[pitch]) {
        wxFont acc_font(N_LINE_SP, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        dc.SetFont(acc_font);
        dc.SetTextForeground(note_col);
        // Unicode ♯ U+266F
        dc.DrawText(wxString::FromUTF8("\xE2\x99\xAF"), nx - head_w - 2, note_y - N_LINE_SP / 2);
    }
}

// ---------------------------------------------------------------------------
// Main draw
// ---------------------------------------------------------------------------

void NotationView::draw(wxDC& dc) {
    wxSize vsz = GetVirtualSize();
    dc.SetBrush(wxBrush(ThemeManager::toWxColour(m_engine.m_tracker_bg)));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(0, 0, vsz.GetWidth(), vsz.GetHeight());

    int num_tracks = (int)m_engine.track_count();
    auto order     = m_engine.order_list();
    int total_rows = get_total_ticks();
    int full_tw    = tick_to_x(total_rows);

    for (int t = 0; t < num_tracks; ++t) {
        auto& track_obj = m_engine.track(t);
        Instrument* inst = track_obj.instrument();
        if (!inst || inst->type() == InstrumentType::Sampler) continue;

        int ty = 30 + t * N_TRACK_H;
        if (ty > vsz.GetHeight()) break;

        // --- Header column ---
        dc.SetBrush(wxBrush(ThemeManager::toWxColour(m_engine.m_bg_color)));
        dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_fg_color)));
        dc.DrawRectangle(0, ty, N_HEADER_W, N_TRACK_H - 1);

        wxFont bold_font(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
        dc.SetFont(bold_font);
        dc.SetTextForeground(ThemeManager::toWxColour(m_engine.m_fg_color));
        dc.DrawText(track_obj.name().substr(0, 14), 5, ty + 5);

        wxFont small_font(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        dc.SetFont(small_font);
        dc.SetTextForeground(ThemeManager::toWxColour(m_engine.m_tracker_text));
        dc.DrawText(inst->name().substr(0, 18), 5, ty + 19);

        const char* type_str = "";
        switch (inst->type()) {
            case InstrumentType::Sampler:   type_str = "[Sampler]";   break;
            case InstrumentType::SoundFont: type_str = "[SoundFont]"; break;
            case InstrumentType::SFZ:       type_str = "[SFZ]";       break;
            case InstrumentType::Plugin:    type_str = "[Plugin]";    break;
            case InstrumentType::Midi:      type_str = "[MIDI]";      break;
            case InstrumentType::Voice:     type_str = "[Voice]";     break;
            default:                        type_str = "[None]";      break;
        }
        dc.DrawText(type_str, 5, ty + 31);

        const char* clef_str = "";
        int notation_type = (int)track_obj.notation();
        switch (notation_type) {
            case 0: clef_str = "Treble";     break;
            case 1: clef_str = "Bass";       break;
            case 2: clef_str = "Grand";      break;
            case 3: clef_str = "Percussion"; break;
            default: clef_str = "?"; break;
        }
        dc.DrawText(clef_str, 5, ty + 43);

        // Divider between header and score area
        dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_text)));
        dc.DrawLine(N_HEADER_W, ty, N_HEADER_W, ty + N_TRACK_H - 1);

        // --- Score area ---
        int staff_type = notation_type;
        int staff_x    = N_HEADER_W;   // staff lines span the full score area
        int notes_x0   = staff_x + N_CLEF_W;  // notes start after clef+time-sig

        // Staff lines and clef (drawn from header edge; clef fits in N_CLEF_W)
        draw_staff(dc, staff_x, ty, full_tw + N_CLEF_W, staff_type);

        // Notes per pattern, with bar lines between patterns
        int rows_done = 0;
        bool first_pat = true;
        for (auto pat_idx : order) {
            auto& pat = m_engine.pattern(pat_idx);
            int pat_rows = (int)pat.row_count();
            int pat_px   = notes_x0 + tick_to_x(rows_done);

            // Bar line at pattern boundary (except before the first pattern)
            if (!first_pat) {
                dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_text)));
                int bar_x = pat_px;
                dc.DrawLine(bar_x, ty + N_STAFF_Y,
                            bar_x, ty + N_STAFF_Y + 4 * N_LINE_SP);
            }
            first_pat = false;

            size_t num_cols = pat.column_count(t);
            for (int r = 0; r < pat_rows; ++r) {
                for (size_t c = 0; c < num_cols; ++c) {
                    const auto& ev = pat.event(t, r, c);
                    if (ev.note < 128) {
                        int nx = pat_px + tick_to_x(r);
                        draw_note(dc, nx, ty, ev.note, staff_type);
                    }
                }
            }
            rows_done += pat_rows;
        }

        // Bottom separator
        dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_fg_color)));
        dc.DrawLine(0, ty + N_TRACK_H - 1, vsz.GetWidth(), ty + N_TRACK_H - 1);
    }

    // --- Playhead ---
    if (m_engine.transport_state() != TransportState::Stopped) {
        int play_x = N_HEADER_W + N_CLEF_W + tick_to_x((int)m_engine.m_current_row);
        dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_cursor)));
        dc.DrawLine(play_x, 0, play_x, vsz.GetHeight());
    }

    // --- Selection overlay ---
    if (m_sel_start_tick != -1 && m_sel_end_tick != -1) {
        int s1  = std::min(m_sel_start_tick, m_sel_end_tick);
        int s2  = std::max(m_sel_start_tick, m_sel_end_tick);
        int sx1 = N_HEADER_W + N_CLEF_W + tick_to_x(s1);
        int sx2 = N_HEADER_W + N_CLEF_W + tick_to_x(s2);
        wxColour sel_col = ThemeManager::toWxColour(m_engine.m_selection_color);
        dc.SetBrush(wxBrush(wxColour(sel_col.Red(), sel_col.Green(), sel_col.Blue(), 64)));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(sx1, 20, sx2 - sx1, vsz.GetHeight() - 20);
    }
}

void NotationView::OnMouseDown(wxMouseEvent& event) {
    int x, y;
    CalcUnscrolledPosition(event.GetX(), event.GetY(), &x, &y);
    if (x > N_HEADER_W + N_CLEF_W) {
        m_is_selecting = true;
        m_sel_start_tick = x_to_tick(x - N_HEADER_W - N_CLEF_W);
        m_sel_end_tick = m_sel_start_tick;
        Refresh();
    }
}

void NotationView::OnMouseDrag(wxMouseEvent& event) {
    if (m_is_selecting) {
        int x, y;
        CalcUnscrolledPosition(event.GetX(), event.GetY(), &x, &y);
        if (x > N_HEADER_W + N_CLEF_W) {
            m_sel_end_tick = x_to_tick(x - N_HEADER_W - N_CLEF_W);
            Refresh();
        }
    }
}

void NotationView::OnMouseUp(wxMouseEvent& event) {
    m_is_selecting = false;
}

void NotationView::OnMouseWheel(wxMouseEvent& event) {
    if (event.ControlDown()) {
        if (event.GetWheelRotation() < 0) zoom_out();
        else zoom_in();
    } else {
        event.Skip();
    }
}

} // namespace disgrace_ns
