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
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Simple_Counter.H>
#include "vu_meter.h"
#include "../core/transport.h" // Add this for TransportState

namespace disgrace_ns
{

// Forward declare Engine within its namespace
class Engine;

class TransportBar : public Fl_Group
{
public:
    TransportBar(int X, int Y, int W, int H, Engine& engine); // Use unqualified Engine now that we are in the namespace

    void update();
    void resize(int x, int y, int w, int h) override;

private:
    static void cb_play(Fl_Widget*, void*);
    static void cb_stop(Fl_Widget*, void*);
    static void cb_record(Fl_Widget*, void*);
    static void cb_metronome(Fl_Widget*, void*);
    static void cb_tempo(Fl_Widget*, void*);
    static void cb_lpb(Fl_Widget*, void*);
    static void cb_step(Fl_Widget*, void*);
    static void cb_loop(Fl_Widget*, void*);

    Engine& m_engine; // Use unqualified Engine

    Fl_Button* m_play;
    Fl_Button* m_stop;
    Fl_Light_Button* m_record;
    Fl_Light_Button* m_metronome;
    Fl_Light_Button* m_loop;
    Fl_Simple_Counter* m_tempo;
    Fl_Simple_Counter* m_lpb;
    Fl_Simple_Counter* m_step;
    Fl_Box* m_status;
    Fl_Box* m_clock;
    VUMeter* m_meter_l;
    VUMeter* m_meter_r;

    char m_clock_str[32];
};

} // namespace disgrace_ns
