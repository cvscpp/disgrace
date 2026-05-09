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

#include "beat_quantizer.h"
#include "timestretch.h"
#include <algorithm>
#include <cmath>

namespace disgrace_ns {

BeatGrid BeatQuantizer::make_metro_grid(float bpm, int sample_rate, int total_samples)
{
    BeatGrid grid;
    grid.sample_rate = sample_rate;

    int period = (int)(sample_rate * 60.0f / bpm);
    if (period <= 0) period = 1;

    for (int pos = 0; pos < total_samples; pos += period)
        grid.beats.push_back(pos);
    grid.beats.push_back(total_samples); // sentinel

    return grid;
}

BeatGrid BeatQuantizer::make_track_grid(const std::vector<Onset>& onsets, int sample_rate)
{
    BeatGrid grid;
    grid.sample_rate = sample_rate;

    if (onsets.empty() || onsets[0].sample_pos > 0)
        grid.beats.push_back(0);

    for (auto& o : onsets)
        grid.beats.push_back(o.sample_pos);

    // ensure sorted
    std::sort(grid.beats.begin(), grid.beats.end());

    return grid;
}

// Find nearest beat in grid to sample position pos
static int nearest_beat(const std::vector<int>& beats, int pos)
{
    if (beats.empty()) return pos;
    auto it = std::lower_bound(beats.begin(), beats.end(), pos);
    if (it == beats.end()) return beats.back();
    if (it == beats.begin()) return beats.front();
    auto prev = std::prev(it);
    return (pos - *prev < *it - pos) ? *prev : *it;
}

std::vector<float> BeatQuantizer::stretch_segment(
    const std::vector<float>& audio,
    size_t src_start, size_t src_end,
    int target_len,
    int sample_rate)
{
    if (src_start >= src_end || target_len <= 0)
        return std::vector<float>(target_len, 0.0f);

    size_t src_len = src_end - src_start;
    std::vector<float> seg(audio.begin() + src_start, audio.begin() + src_end);

    // ratio > 1 means slower (stretching out), < 1 means faster (compressing)
    float ratio = (float)src_len / (float)target_len;
    if (ratio <= 0.0f) ratio = 1.0f;

    std::vector<float> out;
    if (!TimeStretch::stretch(seg, out, ratio, sample_rate) || out.empty()) {
        // Fallback: linear resample
        out.resize(target_len);
        for (int i = 0; i < target_len; ++i) {
            float src_f = (float)i * (float)src_len / (float)target_len;
            size_t idx = (size_t)src_f;
            float frac = src_f - (float)idx;
            float a = (idx < seg.size()) ? seg[idx] : 0.0f;
            float b = (idx + 1 < seg.size()) ? seg[idx + 1] : 0.0f;
            out[i] = a + frac * (b - a);
        }
    }

    // Trim or pad to exact target_len
    out.resize(target_len, 0.0f);
    return out;
}

std::shared_ptr<SampleData> BeatQuantizer::quantize(
    const SampleData& source,
    const std::vector<Onset>& source_onsets,
    const BeatGrid& target_grid,
    float strength)
{
    auto result = std::make_shared<SampleData>();
    result->sample_rate = source.sample_rate;

    const int total = (int)source.left.size();

    // Trivial cases — return unchanged copy
    if (source_onsets.empty() || target_grid.beats.size() < 2) {
        result->left  = source.left;
        result->right = source.right;
        return result;
    }

    // Compute target positions for each onset
    std::vector<int> target_pos;
    target_pos.reserve(source_onsets.size());
    for (auto& o : source_onsets) {
        int nb = nearest_beat(target_grid.beats, o.sample_pos);
        int tp = (int)(o.sample_pos * (1.0f - strength) + nb * strength);
        tp = std::max(0, std::min(tp, total));
        target_pos.push_back(tp);
    }

    // Build segment boundaries
    // src_bounds: [0, onset[0], onset[1], ..., total]
    // dst_bounds: [0, target_pos[0], target_pos[1], ..., total]
    std::vector<int> src_bounds, dst_bounds;
    src_bounds.push_back(0);
    dst_bounds.push_back(0);
    for (size_t i = 0; i < source_onsets.size(); ++i) {
        src_bounds.push_back(source_onsets[i].sample_pos);
        dst_bounds.push_back(target_pos[i]);
    }
    src_bounds.push_back(total);
    dst_bounds.push_back(total);

    const bool stereo = !source.right.empty();

    // Stretch each segment and concatenate
    for (size_t i = 0; i + 1 < src_bounds.size(); ++i) {
        int s0 = src_bounds[i];
        int s1 = src_bounds[i + 1];
        int d0 = dst_bounds[i];
        int d1 = dst_bounds[i + 1];

        if (s0 < 0) s0 = 0;
        if (s1 > total) s1 = total;
        int seg_len = s1 - s0;
        int tgt_len = d1 - d0;

        if (seg_len <= 0 || tgt_len <= 0)
            continue;

        auto left_seg = stretch_segment(source.left, s0, s1, tgt_len, source.sample_rate);
        result->left.insert(result->left.end(), left_seg.begin(), left_seg.end());

        if (stereo) {
            auto right_seg = stretch_segment(source.right, s0, s1, tgt_len, source.sample_rate);
            result->right.insert(result->right.end(), right_seg.begin(), right_seg.end());
        }
    }

    return result;
}

} // namespace disgrace_ns
