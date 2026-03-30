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

#include <cstdint>
#include <cstddef> 

namespace disgrace_ns
{

class Timing
{
public:
    Timing(uint32_t sr = 44100) : m_sample_rate((double)sr) {}

    void set_sample_rate(uint32_t sr) { m_sample_rate = (double)sr; }
    void set_bpm(int bpm); 
    double tempo() const { return (double)m_bpm; } 
    void set_lpb(uint32_t lpb);
    uint32_t lpb() const { return m_lpb; } 
    void set_speed(int speed);

    int bpm() const { return m_bpm; }
    int speed() const { return m_speed; }

    size_t samples_per_tick() const;
    size_t samples_per_row() const;

    size_t samples_per_beat() const;
    size_t samples_per_bar() const;


private:
    double m_sample_rate = 44100.0;
    int m_bpm = 125;     
    int m_speed = 6;     
    uint32_t m_lpb = 4;
};

} // namespace disgrace_ns
