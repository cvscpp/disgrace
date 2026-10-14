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
#include <memory>
#include "voice.h"
#include "sample_data.h"
#include "adsr.h"

namespace disgrace_ns
{

    class SampleVoice : public Voice
    {
    public:
        SampleVoice(std::shared_ptr<disgrace_ns::SampleData> data,
                    double engine_rate);

        void start(uint8_t note,
                   uint8_t velocity,
                   float freq,
                   size_t offset_samples = 0) override;

                   void stop() override;
                   void panic() override;

                   void set_pitch(float freq) override;
                   void set_volume(float vol) override;
                   void set_engine_rate(double sr);
                   void set_sample(std::shared_ptr<disgrace_ns::SampleData> data) { m_sample = data; }

                   // Root note the sample was recorded at (MIDI number) plus
                   // an extra transpose in cents (Renoise Transpose/Finetune;
                   // positive raises playback pitch). Playback speed is
                   // relative to the root, so triggering the root note plays
                   // the sample at its original speed:
                   //   increment = (freq / root_freq) * 2^(cents/1200)
                   //               * (sample_rate / engine_rate)
                   // Default root A4/440 Hz preserves the historical behavior
                   // (samples implicitly rooted at A4) for old projects.
                   void set_root(uint8_t midi_note, int transpose_cents = 0);

                   void process(float* out_l,
                                float* out_r,
                                size_t frames) override;

                                bool active() const override;
                   double position() const { return m_position; }

                   // Set an exclusive end position (0 = whole sample) and optional loop.
                   void set_region(size_t end_pos, bool loop, size_t loop_start) {
                       m_end_pos = end_pos;
                       m_loop_enabled = loop;
                       m_loop_start = loop_start;
                   }
                   // Override ADSR envelope parameters
                   void set_adsr(float attack, float decay, float sustain, float release) {
                       m_env.set(attack, decay, sustain, release);
                   }

    private:
        void update_increment() {
            if (m_sample && m_sample->sample_rate > 0 && m_engine_rate > 0.0 && m_root_freq > 0.0f)
                m_increment = (m_frequency / m_root_freq) * m_transpose_factor *
                              (double(m_sample->sample_rate) / m_engine_rate);
        }

        std::shared_ptr<disgrace_ns::SampleData> m_sample;
        disgrace_ns::ADSR m_env;

        double m_engine_rate;
        double m_position = 0.0;
        double m_increment = 1.0;
        double m_frequency = 440.0;
        float m_root_freq = 440.0f;
        float m_transpose_factor = 1.0f;

        float m_volume = 1.f;
        bool m_active = false;

        size_t m_end_pos = 0;       // 0 = full sample
        bool m_loop_enabled = false;
        size_t m_loop_start = 0;
    };

} // namespace disgrace_ns
