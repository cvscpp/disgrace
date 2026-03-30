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
#include "project_panel.h"

namespace disgrace_ns {

void MainWindow::init_project_tab(int w, int h) {
    m_project_tab = new Fl_Group(0, 65, w, h - 100, "Project");
    m_project_panel = new ProjectPanel(m_project_tab->x(), m_project_tab->y(), m_project_tab->w(), m_project_tab->h(), m_engine);
    m_project_tab->end();
}

} // namespace disgrace_ns
