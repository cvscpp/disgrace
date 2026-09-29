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
#include <cmath>
#include <cstdint>
#include <algorithm>

namespace disgrace_ns
{

    // Channel selection bitmask for destructive sample edits.
    // Bit 0 = left, bit 1 = right.  Used so the sampler editor can apply
    // operations (normalize/gain/silence/fade) to a single stereo channel.
    enum SampleChannel : uint8_t {
        CH_LEFT  = 0x1,
        CH_RIGHT = 0x2,
        CH_BOTH  = 0x3,
    };

    struct SampleData
    {
        ::std::vector<float> left;
        ::std::vector<float> right;
        int sample_rate = 44100;

        void to_mono_l() { right.clear(); }
        void to_mono_r() { if (!right.empty()) left = right; right.clear(); }
        void to_mono_mix() {
            if (right.empty()) return;
            for (size_t i = 0; i < left.size(); ++i) {
                left[i] = (left[i] + right[i]) * 0.5f;
            }
            right.clear();
        }
        void to_stereo() {
            if (!right.empty()) return;
            right = left;
        }

        void normalize(size_t start, size_t end, uint8_t channels = CH_BOTH) {
            end = std::min(end, left.size());
            const bool do_l = (channels & CH_LEFT) != 0;
            const bool do_r = (channels & CH_RIGHT) != 0 && !right.empty();
            if (!do_l && !do_r) return;
            float max_amp = 0.0f;
            for (size_t i = start; i < end; ++i) {
                if (do_l) max_amp = std::max(max_amp, std::abs(left[i]));
                if (do_r) max_amp = std::max(max_amp, std::abs(right[i]));
            }
            if (max_amp < 1e-6f) return;
            float factor = 1.0f / max_amp;
            for (size_t i = start; i < end; ++i) {
                if (do_l) left[i] *= factor;
                if (do_r) right[i] *= factor;
            }
        }

        void adjust_volume(size_t start, size_t end, float factor, uint8_t channels = CH_BOTH) {
            end = std::min(end, left.size());
            const bool do_l = (channels & CH_LEFT) != 0;
            const bool do_r = (channels & CH_RIGHT) != 0 && !right.empty();
            for (size_t i = start; i < end; ++i) {
                if (do_l) left[i] *= factor;
                if (do_r) right[i] *= factor;
            }
        }

        void silence(size_t start, size_t end, uint8_t channels = CH_BOTH) {
            end = std::min(end, left.size());
            const bool do_l = (channels & CH_LEFT) != 0;
            const bool do_r = (channels & CH_RIGHT) != 0 && !right.empty();
            for (size_t i = start; i < end; ++i) {
                if (do_l) left[i] = 0.0f;
                if (do_r) right[i] = 0.0f;
            }
        }

        // NOTE: length-changing edits always keep L/R aligned, so (like cut
        // and paste) insert_silence intentionally ignores any channel mask.
        void insert_silence(size_t pos, size_t len) {
            pos = std::min(pos, left.size());
            left.insert(left.begin() + pos, len, 0.0f);
            if (!right.empty()) {
                right.insert(right.begin() + pos, len, 0.0f);
            }
        }

        void fade_in(size_t start, size_t end, bool log, uint8_t channels = CH_BOTH) {
            end = std::min(end, left.size());
            size_t len = end - start;
            if (len == 0) return;
            const bool do_l = (channels & CH_LEFT) != 0;
            const bool do_r = (channels & CH_RIGHT) != 0 && !right.empty();
            for (size_t i = 0; i < len; ++i) {
                float t = (float)i / (float)len;
                float gain = log ? (powf(10.0f, t) - 1.0f) / 9.0f : t;
                if (do_l) left[start + i] *= gain;
                if (do_r) right[start + i] *= gain;
            }
        }

        void fade_out(size_t start, size_t end, bool log, uint8_t channels = CH_BOTH) {
            end = std::min(end, left.size());
            size_t len = end - start;
            if (len == 0) return;
            const bool do_l = (channels & CH_LEFT) != 0;
            const bool do_r = (channels & CH_RIGHT) != 0 && !right.empty();
            for (size_t i = 0; i < len; ++i) {
                float t = 1.0f - (float)i / (float)len;
                float gain = log ? (powf(10.0f, t) - 1.0f) / 9.0f : t;
                if (do_l) left[start + i] *= gain;
                if (do_r) right[start + i] *= gain;
            }
        }

        SampleData cut(size_t start, size_t end) {
            end = std::min(end, left.size());
            SampleData result;
            result.sample_rate = sample_rate;
            if (start >= end) return result;

            result.left.assign(left.begin() + start, left.begin() + end);
            left.erase(left.begin() + start, left.begin() + end);
            
            if (!right.empty()) {
                result.right.assign(right.begin() + start, right.begin() + end);
                right.erase(right.begin() + start, right.begin() + end);
            }
            return result;
        }

        void paste_at(size_t pos, const SampleData& other) {
            pos = std::min(pos, left.size());
            left.insert(left.begin() + pos, other.left.begin(), other.left.end());
            if (!other.right.empty()) {
                if (right.empty()) right.resize(left.size() - other.left.size(), 0.0f);
                right.insert(right.begin() + pos, other.right.begin(), other.right.end());
            } else if (!right.empty()) {
                right.insert(right.begin() + pos, other.left.size(), 0.0f);
            }
        }
    };

} // namespace disgrace_ns
