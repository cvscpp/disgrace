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

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace disgrace_ns {

class PitchDetector {
public:
    struct Result {
        bool valid = false;
        uint8_t midi_note = 0;
        float frequency = 0.0f;
        float confidence = 0.0f;
    };

    Result estimate_midi_note(const std::vector<float>& mono, int sample_rate,
                              size_t start_sample, size_t end_sample) const
    {
        Result result;
        if (sample_rate <= 0 || start_sample >= end_sample || end_sample > mono.size())
            return result;

        const size_t seg_len = end_sample - start_sample;
        if (seg_len < 256)
            return result;

        size_t window = std::min<size_t>(4096, seg_len);
        if ((window & 1u) != 0u) --window;
        if (window < 256)
            return result;

        const size_t offset = start_sample + (seg_len - window) / 2;
        std::vector<float> frame(window);
        for (size_t i = 0; i < window; ++i) {
            float w = 0.5f - 0.5f * std::cos((2.0f * 3.14159265358979323846f * (float)i) / (float)(window - 1));
            frame[i] = mono[offset + i] * w;
        }

        float energy = 0.0f;
        for (float s : frame) energy += s * s;
        if (energy <= 1e-6f)
            return result;

        const float min_freq = 48.0f;
        const float max_freq = 1800.0f;
        int min_lag = std::max(1, (int)std::floor((float)sample_rate / max_freq));
        int max_lag = std::max(min_lag + 1, (int)std::ceil((float)sample_rate / min_freq));
        max_lag = std::min<int>(max_lag, (int)window / 2);
        if (min_lag >= max_lag)
            return result;

        float best_corr = 0.0f;
        int best_lag = -1;
        for (int lag = min_lag; lag <= max_lag; ++lag) {
            float corr = 0.0f;
            float norm_a = 0.0f;
            float norm_b = 0.0f;
            const size_t limit = window - (size_t)lag;
            for (size_t i = 0; i < limit; ++i) {
                float a = frame[i];
                float b = frame[i + lag];
                corr += a * b;
                norm_a += a * a;
                norm_b += b * b;
            }
            if (norm_a <= 1e-9f || norm_b <= 1e-9f)
                continue;
            corr /= std::sqrt(norm_a * norm_b);
            if (corr > best_corr) {
                best_corr = corr;
                best_lag = lag;
            }
        }

        if (best_lag <= 0 || best_corr < 0.45f)
            return result;

        float frequency = (float)sample_rate / (float)best_lag;
        int midi = (int)std::lround(69.0 + 12.0 * std::log2(frequency / 440.0f));
        if (midi < 0 || midi > 119)
            return result;

        result.valid = true;
        result.midi_note = (uint8_t)midi;
        result.frequency = frequency;
        result.confidence = best_corr;
        return result;
    }
};

} // namespace disgrace_ns
