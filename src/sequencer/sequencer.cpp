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

#include "sequencer.h"

namespace disgrace_ns
{

void Sequencer::set_timing(disgrace_ns::Timing* timing)
{
    m_timing = timing;
}

void Sequencer::set_order_list(const ::std::vector<int>& orders)
{
    m_order_list = orders;
}

void Sequencer::set_patterns(const ::std::vector<disgrace_ns::Pattern>& patterns_list)
{
    for (size_t i = 0; i < patterns_list.size() && i < MAX_PATTERNS; ++i)
    {
        m_patterns[i] = patterns_list[i];
    }
}

const TrackEvent& Sequencer::current_event(size_t track) const
{
    static TrackEvent empty;

    if (m_order_list.empty())
        return empty;

    int pattern_index = m_order_list[m_current_order];
    if (pattern_index < 0 || (size_t)pattern_index >= MAX_PATTERNS) return empty;

    return m_patterns[pattern_index].event(track, m_current_row, 0);
}

void Sequencer::advance_row()
{
    if (m_order_list.empty())
        return;

    int pattern_index = m_order_list[m_current_order];
    if (pattern_index < 0 || (size_t)pattern_index >= MAX_PATTERNS) return;

    m_current_row++;

    if (m_current_row >= m_patterns[pattern_index].row_count())
    {
        m_current_row = 0;
        m_current_order++;

        if (m_current_order >= m_order_list.size())
            m_current_order = 0;
    }

    if (m_pending_order_jump >= 0)
    {
        m_current_order = m_pending_order_jump;
        m_current_row = 0;
        m_pending_order_jump = -1;
    }
} 

void Sequencer::jump_to_order(int order)
{
    m_pending_order_jump = order;
}

void Sequencer::break_to_row(int row)
{
    m_pending_row_break = row;
}

void Sequencer::pattern_break()
{
    m_current_row = 0;
    m_current_order++;
}

void Sequencer::jump_to(size_t order)
{
    if (order < m_order_length)
    {
        m_current_order = order;
        m_current_row = 0;
    }
}

} // namespace disgrace_ns
