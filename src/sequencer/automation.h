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
#include <array>
#include <cstddef> // Added for size_t

namespace disgrace_ns
{

    constexpr size_t MAX_AUTOMATION_POINTS = 512;

    struct AutomationPoint
    {
        size_t row;
        float value;
    };

    class AutomationLane
    {
    public:
        void add_point(size_t row, float value);
        float value_at(size_t row) const;

    private:
        ::std::array<disgrace_ns::AutomationPoint,
        MAX_AUTOMATION_POINTS> m_points{};
        size_t m_count = 0;
    };

} // namespace disgrace_ns
