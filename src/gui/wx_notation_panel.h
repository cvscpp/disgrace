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

#pragma once

#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/button.h>
#include <wx/sizer.h>

namespace disgrace_ns {

class Engine;

class NotationView : public wxScrolledWindow {
public:
    NotationView(wxWindow* parent, wxWindowID id, Engine& engine);
    void zoom_in();
    void zoom_out();
    void view_all();
    void view_selection();
    void update_view();
    void OnPaint(wxPaintEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseDrag(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);
    void OnSize(wxSizeEvent& event);
    void draw(wxDC& dc);

private:
    void draw_staff(wxDC& dc, int tx, int ty, int tw, int type);
    void draw_note(wxDC& dc, int nx, int ny, int note, int staff_type);
    void draw_clef_treble(wxDC& dc, int x, int y);
    void draw_clef_bass(wxDC& dc, int x, int y);
    void on_preview_track(wxCommandEvent& event);

    int get_total_ticks();
    int tick_to_x(int tick);
    int x_to_tick(int x);

    Engine& m_engine;
    double m_zoom;
    int m_sel_start_tick = -1;
    int m_sel_end_tick = -1;
    bool m_is_selecting = false;
    bool m_needs_initial_view_all = true;

    std::vector<wxButton*> m_preview_buttons;

    wxDECLARE_EVENT_TABLE();
};

class NotationPanel : public wxPanel {
public:
    NotationPanel(wxWindow* parent, Engine& engine);
    void update();

public:
    void set_tab_index(int idx) { m_tab_index = idx; }

private:
    void on_zoom_in(wxCommandEvent& event);
    void on_zoom_out(wxCommandEvent& event);
    void on_view_all(wxCommandEvent& event);
    void on_view_sel(wxCommandEvent& event);
    void on_preview_all(wxCommandEvent& event);
    void on_detach(wxCommandEvent& event);

    Engine& m_engine;
    int m_tab_index = -1;
    size_t m_last_track_count = 0;
    wxButton* m_zoom_in_btn;
    wxButton* m_zoom_out_btn;
    wxButton* m_view_all_btn;
    wxButton* m_view_sel_btn;
    wxButton* m_preview_all_btn;
    wxButton* m_detach_btn;
    class NotationView* m_notation_view;
    class DetachedFrame* m_detached_frame = nullptr;

    wxDECLARE_EVENT_TABLE();
};

} // namespace disgrace_ns
