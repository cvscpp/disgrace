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

#include <FL/Fl_Group.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <vector>

namespace disgrace_ns {

class Engine;

class TracksView : public Fl_Widget {
public:
    TracksView(int x, int y, int w, int h, Engine& engine);
    void draw() override;
    int handle(int event) override;

    void zoom_in();
    void zoom_out();
    void view_all();
    void view_selection();

    void update_view();

private:
    Engine& m_engine;
    double m_zoom = 1.0; // horizontal zoom (pixels per tick or row)
    int m_scroll_x = 0;
    
    int m_sel_start_tick = -1;
    int m_sel_end_tick = -1;
    bool m_is_selecting = false;

    int get_total_ticks();
    int tick_to_x(int tick);
    int x_to_tick(int x);
};

class TracksPanel : public Fl_Group {
public:
    TracksPanel(int x, int y, int w, int h, Engine& engine);
    void update();

private:
    Engine& m_engine;
    Fl_Scroll* m_scroll;
    TracksView* m_tracks_view;

    Fl_Button* m_zoom_in_btn;
    Fl_Button* m_zoom_out_btn;
    Fl_Button* m_view_all_btn;
    Fl_Button* m_view_sel_btn;

    static void cb_zoom_in(Fl_Widget*, void*);
    static void cb_zoom_out(Fl_Widget*, void*);
    static void cb_view_all(Fl_Widget*, void*);
    static void cb_view_sel(Fl_Widget*, void*);
};

} // namespace disgrace_ns
