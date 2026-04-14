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
#include <fluidsynth.h>
#include <vector>
#include <string>

namespace disgrace_ns
{

    struct SoundFontPreset {
        int bank;
        int num;
        std::string name;
    };

    class SoundFontInstrument : public disgrace_ns::Instrument
    {
    public:
        SoundFontInstrument(double sample_rate);
        ~SoundFontInstrument();

            void note_on(uint8_t note, uint8_t velocity, size_t column_index = 0, size_t offset_samples = 0, uint8_t sample_index = 0) override;
            void note_off(size_t column_index = 0) override;
            void panic() override;
            void set_volume(float vol) override;
        float get_volume() const { return m_volume; }
        
        void set_pitch(float freq) override;
        void process(float* l, float* r, size_t nframes) override;

        bool load_soundfont(const std::string& path);
        const std::vector<SoundFontPreset>& presets() const { return m_presets; }
        const std::string& path() const { return m_path; }
        void set_preset(int index);
        int current_preset() const { return m_selected_preset; }

    protected:
        ::std::unique_ptr<disgrace_ns::Voice> create_voice() override { return nullptr; } // FluidSynth handles voices

    private:
        fluid_settings_t* m_fluid_settings;
        fluid_synth_t* m_fluid_synth;
        int m_sfont_id = -1;
        std::vector<SoundFontPreset> m_presets;
        int m_selected_preset = -1;
        float m_volume = 1.0f;
        std::string m_path;
        int m_last_note[16]{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    };

} // namespace disgrace_ns
