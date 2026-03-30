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

#include "automation.h"
#include <algorithm>

namespace disgrace_ns
{

    void disgrace_ns::AutomationLane::add_point(size_t row, float value)
    {
        if (m_count >= MAX_AUTOMATION_POINTS)
            return;

        m_points[m_count++] = { row, value };

        ::std::sort(m_points.begin(),
                  m_points.begin() + m_count,
                  [](const disgrace_ns::AutomationPoint& a,
                     const disgrace_ns::AutomationPoint& b)
                  {
                      return a.row < b.row;
                  });
    }

    float disgrace_ns::AutomationLane::value_at(size_t row) const
    {
        if (m_count == 0)
            return 0.0f;

        if (m_count == 1)
            return m_points[0].value;

        if (row <= m_points[0].row)
            return m_points[0].value;

        if (row >= m_points[m_count - 1].row)
            return m_points[m_count - 1].value;

        for (size_t i = 0; i < m_count - 1; ++i)
        {
            const auto& a = m_points[i];
            const auto& b = m_points[i + 1];

            if (row >= a.row && row <= b.row)
            {
                float t =
                float(row - a.row) /
                float(b.row - a.row);

                return a.value +
                t * (b.value - a.value);
            }
        }

        return 0.0f;
    }

} // namespace disgrace_ns
