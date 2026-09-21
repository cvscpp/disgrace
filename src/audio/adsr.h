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

namespace disgrace_ns
{

    class ADSR
    {
    public:
        enum class Stage
        {
            Idle,
            Attack,
            Decay,
            Sustain,
            Release
        };

        void set_sample_rate(double sr)
        {
            m_sample_rate = std::max(1.0, sr);
        }

        void set(float attack,
                 float decay,
                 float sustain,
                 float release)
        {
            m_attack  = std::max(0.0f, attack);
            m_decay   = std::max(0.0f, decay);
            m_sustain = std::clamp(sustain, 0.0f, 1.0f);
            m_release = std::max(0.0f, release);
        }

        void note_on()
        {
            m_stage = Stage::Attack;
            m_level = 0.f;
            m_release_start_level = 0.f;
        }

        void note_off()
        {
            if (m_stage != Stage::Idle) {
                m_release_start_level = m_level;
                m_stage = Stage::Release;
                if (m_release <= 0.0f || m_release_start_level <= 0.0f)
                    reset();
            }
        }

        void reset()
        {
            m_stage = Stage::Idle;
            m_level = 0.f;
            m_release_start_level = 0.f;
        }

        float process()
        {
            switch (m_stage)
            {
                case Stage::Idle:
                    return 0.f;

                case Stage::Attack:
                    if (m_attack <= 0.0f) {
                        m_level = 1.f;
                        m_stage = Stage::Decay;
                        break;
                    }
                    m_level += 1.0f / (m_attack * m_sample_rate);

                    if (m_level >= 1.f)
                    {
                        m_level = 1.f;
                        m_stage = Stage::Decay;
                    }
                    break;

                case Stage::Decay:
                    if (m_decay <= 0.0f) {
                        m_level = m_sustain;
                        m_stage = Stage::Sustain;
                        break;
                    }
                    m_level -= (1.f - m_sustain) / (m_decay * m_sample_rate);

                    if (m_level <= m_sustain)
                    {
                        m_level = m_sustain;
                        m_stage = Stage::Sustain;
                    }
                    break;

                case Stage::Sustain:
                    break;

                case Stage::Release:
                    if (m_release <= 0.0f) {
                        reset();
                        return 0.f;
                    }
                    m_level -= m_release_start_level / (m_release * m_sample_rate);

                    if (m_level <= 0.f)
                    {
                        m_level = 0.f;
                        m_stage = Stage::Idle;
                    }
                    break;
            }

            return m_level;
        }

        bool active() const
        {
            return m_stage != Stage::Idle;
        }

    private:
        double m_sample_rate = 44100.0;

        float m_attack  = 0.01f;
        float m_decay   = 0.1f;
        float m_sustain = 0.8f;
        float m_release = 0.2f;

        float m_level = 0.f;
        float m_release_start_level = 0.f;
        Stage m_stage = Stage::Idle;
    };

} // namespace disgrace_ns
