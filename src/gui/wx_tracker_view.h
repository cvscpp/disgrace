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

#include <wx/wxprec.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <vector>
#include <cstdint>

#include "../sequencer/pattern.h"
#include "../core/key_bindings.h"

namespace disgrace_ns {

class Engine;

class TrackerView : public wxScrolledWindow {
public:
    TrackerView(wxWindow* parent, wxWindowID id, Pattern& pattern, Engine& engine);

    void draw(wxDC& dc);
    void OnPaint(wxPaintEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseDrag(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnRightClick(wxMouseEvent& event);
    void do_transpose(int semitones);
    void do_transpose_pattern(int semitones);
    void do_transpose_song(int semitones);
    // Merge the cursor track's subtracks into the adjacent track in the
    // given direction (-1: previous, +1: next) and delete the cursor track.
    void do_join_track(int direction);

    void set_current_row(int row);
    int get_cursor_row() const { return m_cursor_row; }
    int get_cursor_track() const { return m_cursor_track; }
    int get_cursor_col() const { return m_cursor_col; }
    int get_cursor_field() const { return m_cursor_field; }
    void set_cursor_track(int track) { m_cursor_track = track; clamp_cursor(); }
    void set_pattern(Pattern& pattern);
    void recalculate_size();
    void ensure_cursor_visible();
    bool handle_action(Action action);

    struct TrackUI {
        int x, w;
        int btn_plus_x, btn_minus_x;
    };

private:
    void on_text_enter(wxCommandEvent& event);
    void on_text_kill_focus(wxFocusEvent& event);
    void start_text_edit();
    void stop_text_edit(bool apply = true);

    int get_field_at(int track, int x);
    int get_field_x(int track, int abs_field, int& width);
    void delete_current_field();
    void clamp_cursor();
    // Push the current cursor track to the engine so live MIDI input plays
    // through (and records into) the selected track.
    void sync_record_track();
    void insert_note(uint8_t note);
    int get_center_row_y();

    struct GridColours {
        wxColour bg, text, note, sample, sample_dim, effect, volume, lpb, cursor;
    };
    struct RowCache {
        wxBitmap bmp;
        uint64_t stamp = 0;
        uint32_t style = 0;
        int width = 0;
    };
    void clear_row_cache();
    uint32_t compute_style_stamp(const GridColours& col, uint32_t lpb) const;
    uint64_t compute_row_stamp(int row) const;
    bool row_is_hot(int row, int playing_row, bool is_playing) const;
    void draw_track_cells(wxDC& dc, int track, int row, int ry, bool overlay_only, const GridColours& col);
    void ensure_row_cache(int row, int width, const GridColours& col, uint32_t style, uint32_t lpb);

    std::vector<RowCache> m_row_cache;

    Engine& m_engine;
    Pattern* m_pattern;

    std::vector<TrackUI> m_track_ui;

    int m_cursor_row = 0;
    int m_cursor_track = 0;
    int m_cursor_col = 0;
    int m_cursor_field = 0; // 0: Note, 1: Sample, 2: Volume, 3: FX1, 4: P1, 5: FX2, 6: P2

    wxTextCtrl* m_text_editor = nullptr;
    bool m_is_editing_text = false;
    bool m_preview_active = false;
    int m_preview_track = -1;
    int m_preview_col = -1;
    uint8_t m_preview_note = 255;

    struct TrackerPos {
        int track;
        int row;
        int field; // Absolute field index in track

        TrackerPos(int t = -1, int r = -1, int f = -1) : track(t), row(r), field(f) {}
        bool operator==(const TrackerPos& o) const { return track == o.track && row == o.row && field == o.field; }
    };

    TrackerPos m_sel_start = {-1, -1, -1};
    TrackerPos m_sel_end = {-1, -1, -1};
    bool m_sel_active = false;
    bool m_selecting = false;

    wxDECLARE_EVENT_TABLE();
};

} // namespace disgrace_ns
