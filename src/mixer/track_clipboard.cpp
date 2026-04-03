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

#include "track_clipboard.h"

namespace disgrace_ns {

void TrackClipboard::set_audio(const std::vector<float>& left, const std::vector<float>& right) {
    m_left = left;
    m_right = right;
}

bool TrackClipboard::get_audio(std::vector<float>& left, std::vector<float>& right) const {
    if (m_left.empty() || m_right.empty()) {
        return false;
    }
    left = m_left;
    right = m_right;
    return true;
}

bool TrackClipboard::has_content() const {
    return !m_left.empty() && !m_right.empty();
}

void TrackClipboard::clear() {
    m_left.clear();
    m_right.clear();
}

size_t TrackClipboard::sample_count() const {
    return m_left.size();
}

} // namespace disgrace_ns
