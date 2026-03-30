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
                   void set_sample(std::shared_ptr<disgrace_ns::SampleData> data) { m_sample = data; }

                   void process(float* out_l,
                                float* out_r,
                                size_t frames) override;

                                bool active() const override;

    private:
        std::shared_ptr<disgrace_ns::SampleData> m_sample;
        disgrace_ns::ADSR m_env;

        double m_engine_rate;
        double m_position = 0.0;
        double m_increment = 1.0;

        float m_volume = 1.f;
        bool m_active = false;
    };

} // namespace disgrace_ns
