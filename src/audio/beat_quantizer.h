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
#include <memory>
#include "sample_data.h"
#include "../analysis/onset_detector.h"

namespace disgrace_ns {

struct BeatGrid {
    std::vector<int> beats;  // beat positions in samples
    int sample_rate = 44100;
};

class BeatQuantizer {
public:
    // Build a regular metronome grid for total_samples of audio at bpm BPM.
    static BeatGrid make_metro_grid(float bpm, int sample_rate, int total_samples);

    // Build a grid from detected onsets of a reference SampleData.
    static BeatGrid make_track_grid(const std::vector<Onset>& onsets, int sample_rate);

    // Warp source so its onsets snap toward target_grid beats.
    // strength: 0.0 = no warp, 1.0 = full snap.
    static std::shared_ptr<SampleData> quantize(
        const SampleData& source,
        const std::vector<Onset>& source_onsets,
        const BeatGrid& target_grid,
        float strength = 1.0f);

private:
    static std::vector<float> stretch_segment(
        const std::vector<float>& audio,
        size_t src_start, size_t src_end,
        int target_len,
        int sample_rate);
};

} // namespace disgrace_ns
