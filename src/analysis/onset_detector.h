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

namespace disgrace_ns {

struct SampleData;

struct Onset {
    int sample_pos;  // position in samples within the SampleData
    float strength;  // relative energy increase, 0..1
};

class OnsetDetector {
public:
    // threshold: relative delta-energy threshold, 0..1, default 0.35
    // min_gap_ms: minimum gap between onsets in ms, default 80
    explicit OnsetDetector(float threshold = 0.35f, float min_gap_ms = 80.0f);

    void set_threshold(float t) { m_threshold = t; }
    void set_min_gap_ms(float ms) { m_min_gap_ms = ms; }

    // Detect onsets from SampleData (uses mono mix of left+right)
    std::vector<Onset> detect(const SampleData& data) const;

private:
    float m_threshold;
    float m_min_gap_ms;

    static constexpr int HOP   = 256;
    static constexpr int FRAME = 512;
};

} // namespace disgrace_ns
