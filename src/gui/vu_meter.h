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

#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

namespace disgrace_ns {

class Engine;

class VUMeter : public Fl_Box {
public:
    VUMeter(int x, int y, int w, int h, Engine& engine, const char* label = nullptr, bool horizontal = false);

    void level(float l);
    float level() const { return m_level; }

    void draw() override;

private:
    Engine& m_engine;
    float m_level = 0.0f;
    float m_peak_hold = 0.0f;
    int m_peak_timer = 0;
    bool m_horizontal = false;
};

} // namespace disgrace_ns
