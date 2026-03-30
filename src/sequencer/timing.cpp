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

#include "timing.h"
#include <cstddef>

namespace disgrace_ns
{

    void Timing::set_bpm(int bpm)
    {
        if (bpm > 10 && bpm < 400)
            m_bpm = bpm;
    }

    void Timing::set_lpb(uint32_t lpb)
    {
        if (lpb > 0 && lpb < 128)
            m_lpb = lpb;
    }

    void Timing::set_speed(int speed)
    {
        if (speed > 0 && speed < 32)
            m_speed = speed;
    }

    size_t Timing::samples_per_tick() const
    {
        double tick_sec = 2.5 / double(m_bpm);
        return static_cast<size_t>(m_sample_rate * tick_sec);
    }

    size_t Timing::samples_per_row() const
    {
        return samples_per_tick() * m_speed;
    }
    size_t Timing::samples_per_beat() const
    {
        return samples_per_row() * m_lpb;
    }

    size_t Timing::samples_per_bar() const
    {
        return samples_per_beat() * 4;
    }

} // namespace disgrace_ns
