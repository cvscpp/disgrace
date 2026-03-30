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

#include "main_window.h"
#include "tracks_panel.h"

namespace disgrace_ns {

void MainWindow::init_tracks_tab(int w, int h) {
    m_tracks_tab = new Fl_Group(0, 65, w, h - 100, "Tracks");
    m_tracks_panel = new TracksPanel(m_tracks_tab->x(), m_tracks_tab->y(), m_tracks_tab->w(), m_tracks_tab->h(), m_engine);
    m_tracks_tab->end();
}

} // namespace disgrace_ns
