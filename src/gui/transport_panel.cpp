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

#include "transport_panel.h"
#include "../core/engine.h"

namespace disgrace_ns
{

    disgrace_ns::TransportPanel::TransportPanel(int x, int y,
                                   int w, int h,
                                   disgrace_ns::Engine& engine)
    : Fl_Group(x, y, w, h),
    m_engine(engine)
    {
        begin();

        m_play = new Fl_Button(x + 10, y + 5, 60, 30, "Play");
        m_play->callback(cb_play, this);

        m_stop = new Fl_Button(x + 80, y + 5, 60, 30, "Stop");
        m_stop->callback(cb_stop, this);

        m_tempo = new Fl_Value_Input(x + 160, y + 5, 80, 30, "BPM");
        m_tempo->value(125);
        m_tempo->callback(cb_tempo, this);

        end();
    }

    void disgrace_ns::TransportPanel::cb_play(Fl_Widget*, void* userdata)
    {
        auto* self = static_cast<disgrace_ns::TransportPanel*>(userdata);
        self->m_engine.play();
    }

    void disgrace_ns::TransportPanel::cb_stop(Fl_Widget*, void* userdata)
    {
        auto* self = static_cast<disgrace_ns::TransportPanel*>(userdata);
        self->m_engine.stop();
    }

    void disgrace_ns::TransportPanel::cb_tempo(Fl_Widget*, void* userdata)
    {
        auto* self = static_cast<disgrace_ns::TransportPanel*>(userdata);
        self->m_engine.set_tempo(self->m_tempo->value());
    }

} // namespace disgrace_ns
