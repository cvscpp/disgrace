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

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Value_Input.H>

namespace disgrace_ns
{

    class Engine;

    class TransportPanel : public Fl_Group
    {
    public:
        TransportPanel(int x, int y, int w, int h, disgrace_ns::Engine& engine);

    private:
        static void cb_play(Fl_Widget*, void*);
        static void cb_stop(Fl_Widget*, void*);
        static void cb_tempo(Fl_Widget*, void*);

        disgrace_ns::Engine& m_engine;

        Fl_Button*      m_play;
        Fl_Button*      m_stop;
        Fl_Value_Input* m_tempo;
    };

} // namespace disgrace_ns
