#pragma once

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

#include <FL/Fl_Double_Window.H>

namespace disgrace_ns {

class TrackerPanel; // Forward declaration

class DetachedWindow : public Fl_Double_Window {
public:
    DetachedWindow(int w, int h, const char* title, Fl_Group* panel, Fl_Group* original_parent);

private:
    Fl_Group* m_panel;
    Fl_Group* m_original_parent;

    static void close_cb(Fl_Widget*, void*);
    void handle_close();
};

} // namespace disgrace_ns
