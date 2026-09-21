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

#include "sample_voice.h"
#include <algorithm>
#include <cmath>

namespace disgrace_ns
{

    disgrace_ns::SampleVoice::SampleVoice(std::shared_ptr<disgrace_ns::SampleData> data,
                             double engine_rate)
    : m_sample(data),
    m_engine_rate(engine_rate)
    {
        m_env.set_sample_rate(engine_rate);
        m_env.set(0.005f, 0.05f, 0.9f, 0.2f);
    }

    void disgrace_ns::SampleVoice::start(uint8_t note,
                            uint8_t velocity,
                            float freq,
                            size_t offset_samples)
    {
        (void)note;
        if (!m_sample || m_sample->left.empty() ||
            m_sample->sample_rate <= 0 || m_engine_rate <= 0.0) {
            m_active = false;
            m_env.reset();
            return;
        }

        m_position = (double)offset_samples;
        m_frequency = std::max(0.0, (double)freq);
        m_increment = (m_frequency / 440.0) *
                      (double(m_sample->sample_rate) / m_engine_rate);

        m_volume = velocity / 127.f;
         m_env.note_on();
        m_active = true;
    }

    void disgrace_ns::SampleVoice::stop()
    {
         m_env.note_off();
    }

    void disgrace_ns::SampleVoice::panic()
    {
        m_env.reset();
        m_active = false;
    }

    void disgrace_ns::SampleVoice::set_pitch(float freq)
    {
        m_frequency = std::max(0.0, (double)freq);
        if (m_sample && m_sample->sample_rate > 0 && m_engine_rate > 0.0)
            m_increment = (m_frequency / 440.0) *
                          (double(m_sample->sample_rate) / m_engine_rate);
    }

    void disgrace_ns::SampleVoice::set_volume(float vol)
    {
        m_volume = vol;
    }

    void disgrace_ns::SampleVoice::set_engine_rate(double sr)
    {
        m_engine_rate = std::max(1.0, sr);
        m_env.set_sample_rate(sr);
        if (m_sample && m_sample->sample_rate > 0)
            m_increment = (m_frequency / 440.0) *
                          (double(m_sample->sample_rate) / m_engine_rate);
    }

    bool disgrace_ns::SampleVoice::active() const
    {
        return m_active;
    }


    void disgrace_ns::SampleVoice::process(float* out_l,
                              float* out_r,
                              size_t frames)
    {
        if (!m_env.active() || !m_sample)
        {
            m_active = false;
            return;
        }

            const size_t sample_size = m_sample->left.size();
            if (sample_size == 0) {
                m_active = false;
                return;
            }

            const size_t effective_end =
                (m_end_pos > 0) ? std::min(m_end_pos, sample_size) : sample_size;
            if (effective_end == 0) {
                m_active = false;
                return;
            }

            for (size_t i = 0; i < frames; ++i)
            {
                if (m_position >= (double)effective_end)
                {
                    if (m_loop_enabled) {
                        m_position = (double)std::min(m_loop_start, effective_end - 1);
                    } else {
                        m_active = false;
                        return;
                    }
                }

                size_t idx = std::min(size_t(m_position), effective_end - 1);
                float frac = float(m_position - idx);

                // Linear interpolation
                float l0 = m_sample->left[idx];
                size_t next = std::min(idx + 1, sample_size - 1);
                float l1 = m_sample->left[next];
                float l  = l0 + (l1 - l0) * frac;

            float r = l;
            if (!m_sample->right.empty())
            {
                float r0 = m_sample->right[idx];
                float r1 = m_sample->right[std::min(next, m_sample->right.size() - 1)];
                r = r0 + (r1 - r0) * frac;
            }

            float env = m_env.process();

            out_l[i] += l * m_volume * env;
            out_r[i] += r * m_volume * env;

            m_position += m_increment;
        }

        if (!m_env.active())
            m_active = false;
    }



} // namespace disgrace_ns
