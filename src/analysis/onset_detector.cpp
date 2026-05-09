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

#include "onset_detector.h"
#include "../audio/sample_data.h"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace disgrace_ns {

OnsetDetector::OnsetDetector(float threshold, float min_gap_ms)
    : m_threshold(threshold), m_min_gap_ms(min_gap_ms)
{
}

std::vector<Onset> OnsetDetector::detect(const SampleData& data) const
{
    if (data.left.empty())
        return {};

    const int sr = data.sample_rate;
    const bool stereo = !data.right.empty();
    const size_t n = data.left.size();

    // Build mono mix
    std::vector<float> mono(n);
    if (stereo) {
        for (size_t i = 0; i < n; ++i)
            mono[i] = (data.left[i] + data.right[i]) * 0.5f;
    } else {
        mono = data.left;
    }

    // Compute RMS energy per hop frame
    std::vector<float> energy;
    energy.reserve(n / HOP + 1);
    for (size_t pos = 0; pos < n; pos += HOP) {
        size_t end = std::min(pos + (size_t)FRAME, n);
        float sum = 0.0f;
        for (size_t i = pos; i < end; ++i)
            sum += mono[i] * mono[i];
        energy.push_back(std::sqrt(sum / (float)(end - pos)));
    }

    if (energy.size() < 2)
        return {};

    // Half-wave rectified spectral flux (delta)
    std::vector<float> delta(energy.size(), 0.0f);
    for (size_t i = 1; i < energy.size(); ++i)
        delta[i] = std::max(0.0f, energy[i] - energy[i - 1]);

    // Adaptive threshold: median of deltas * (1 + threshold * 3)
    std::vector<float> sorted_delta = delta;
    std::sort(sorted_delta.begin(), sorted_delta.end());
    float median = sorted_delta[sorted_delta.size() / 2];
    float adaptive_threshold = median * (1.0f + m_threshold * 3.0f);

    // Minimum gap between onsets in frames
    int min_gap_frames = std::max(1, (int)(m_min_gap_ms * sr / 1000.0f / HOP));

    // Pick onsets
    std::vector<Onset> onsets;
    int last_onset_frame = -min_gap_frames;
    for (size_t i = 1; i < delta.size(); ++i) {
        if (delta[i] > adaptive_threshold &&
            (int)i - last_onset_frame >= min_gap_frames)
        {
            Onset o;
            o.sample_pos = (int)(i * HOP);
            o.strength   = delta[i]; // normalized below
            onsets.push_back(o);
            last_onset_frame = (int)i;
        }
    }

    // Normalize strengths by max delta
    float max_delta = 0.0f;
    for (auto& o : onsets) max_delta = std::max(max_delta, o.strength);
    if (max_delta > 0.0f)
        for (auto& o : onsets) o.strength /= max_delta;

    return onsets;
}

} // namespace disgrace_ns
