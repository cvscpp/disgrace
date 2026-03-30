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

#include "pattern.h"
#include "timing.h"
#include <array>
#include <vector>

namespace disgrace_ns
{

constexpr size_t MAX_PATTERNS = 128;

class Sequencer
{
public:
    void set_timing(disgrace_ns::Timing* timing);

    void advance_row();
    void jump_to_order(int order);
    void break_to_row(int row);
    void pattern_break(); 
    void jump_to(size_t order); 

    size_t current_order() const { return m_current_order; }
    size_t current_row() const { return m_current_row; }
    void set_order_list(const ::std::vector<int>& orders);
    void set_patterns(const ::std::vector<disgrace_ns::Pattern>& patterns_list);


    const TrackEvent& current_event(size_t track) const;

    static constexpr size_t MAX_ORDER = 256;

    ::std::array<uint8_t, MAX_ORDER> m_order{};
    size_t m_order_length = 1;

    size_t m_current_order = 0;

private:
    ::std::vector<int> m_order_list;

    int m_pending_order_jump = -1;
    int m_pending_row_break = -1;
    disgrace_ns::Timing* m_timing = nullptr;

    ::std::array<disgrace_ns::Pattern, MAX_PATTERNS> m_patterns{};

    size_t m_current_pattern = 0;
    size_t m_current_row = 0;
};

} // namespace disgrace_ns
