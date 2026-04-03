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

#include <vector>
#include <cstddef>

namespace disgrace_ns {

/**
 * Track Audio Clipboard
 * 
 * Stores audio samples (L/R channels) copied from tracks.
 * Shared per-engine clipboard for copy/paste operations.
 */
class TrackClipboard {
public:
    TrackClipboard() = default;

    /**
     * Copy audio samples to clipboard
     * @param left Left channel samples
     * @param right Right channel samples
     */
    void set_audio(const std::vector<float>& left, const std::vector<float>& right);

    /**
     * Get clipboard content
     * @param left Output: left channel samples
     * @param right Output: right channel samples
     * @return true if clipboard has content, false if empty
     */
    bool get_audio(std::vector<float>& left, std::vector<float>& right) const;

    /**
     * Check if clipboard has audio
     */
    bool has_content() const;

    /**
     * Clear clipboard
     */
    void clear();

    /**
     * Get number of samples in clipboard
     */
    size_t sample_count() const;

private:
    std::vector<float> m_left;
    std::vector<float> m_right;
};

} // namespace disgrace_ns
