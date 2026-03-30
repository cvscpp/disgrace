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

#include <FL/Fl_Widget.H>
#include <memory>
#include "../audio/sample_data.h"
#include "FL/Fl_File_Chooser.H"

namespace disgrace_ns
{

    enum class ChannelMode { Both, Left, Right };

    class Engine;

    class WaveformView : public Fl_Widget
    {
    public:
        WaveformView(int x, int y, int w, int h, Engine& engine);

        void set_sample(::std::shared_ptr<disgrace_ns::SampleData> s);
        void set_color(unsigned int c) { m_color = c; redraw(); }
        
        void draw() override;
        int handle(int event) override;

        void zoom_in();
        void zoom_out();
        void view_all();
        void view_selection();

        void set_channel_mode(ChannelMode mode) { m_mode = mode; redraw(); }
        
        size_t selection_start() const { return m_sel_start; }
        size_t selection_end() const { return m_sel_end; }

    private:
        Engine& m_engine;
        ::std::shared_ptr<disgrace_ns::SampleData> m_sample;
        unsigned int m_color = 0x40FF4000;
        
        size_t m_sel_start = 0;
        size_t m_sel_end = 0;
        
        double m_zoom = 1.0;
        size_t m_offset = 0;
        
        ChannelMode m_mode = ChannelMode::Both;

        void get_view_range(size_t& start, size_t& end);
    };

} // namespace disgrace_ns
