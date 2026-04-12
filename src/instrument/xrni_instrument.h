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
#include "instrument.h"
#include "sfz_instrument.h"   // reuse SfzRegion / SfzVoice
#include "../audio/sample_voice.h"
#include "../audio/sample_data.h"
#include <vector>
#include <string>
#include <memory>

namespace disgrace_ns
{

class XrniInstrument : public Instrument
{
public:
    explicit XrniInstrument(double sample_rate);
    ~XrniInstrument() override = default;

    void note_on(uint8_t note, uint8_t velocity, size_t column_index = 0,
                 size_t offset_samples = 0, uint8_t sample_index = 0) override;
    void note_off(size_t column_index = 0) override;
    void panic() override;
    void set_volume(float vol) override;
    float get_volume() const { return m_volume; }
    void set_pitch(float freq) override {}
    void process(float* l, float* r, size_t nframes) override;

    bool load_xrni(const std::string& path);
    const std::string& path() const { return m_path; }
    const std::string& name() const { return m_xrni_name; }

    // Sample list (one entry per <Sample> in the XRNI)
    const std::vector<std::string>& sample_names() const { return m_sample_names; }

protected:
    std::unique_ptr<Voice> create_voice() override { return nullptr; }

private:
    static int parse_note(const std::string& s);
    static float midi_note_to_hz(int note);

    double m_sample_rate;
    std::string m_path;
    std::string m_xrni_name;

    std::vector<SfzRegion>   m_regions;
    std::vector<std::string> m_sample_names;

    std::vector<SfzVoice> m_voices;
    static constexpr size_t MAX_VOICES = 48;

    float m_volume = 1.0f;
};

} // namespace disgrace_ns
