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
#include <map>
#include "instrument.h"
#include "../audio/sample_voice.h"

namespace disgrace_ns
{

    struct SampleEntry {
        std::string name;
        std::shared_ptr<disgrace_ns::SampleData> data;
    };

    enum class SampleFormatAction {
        Stereo,
        StereoToMonoL,
        StereoToMonoR,
        StereoToMonoMix,
        MonoToStereo
    };

    class SampleInstrument : public disgrace_ns::Instrument
    {
    public:
        SampleInstrument(double engine_rate);

        void note_on(uint8_t note, uint8_t velocity, size_t column_index = 0, size_t offset_samples = 0, uint8_t sample_index = 0) override;
        void note_off(size_t column_index = 0) override;
        void panic() override;
        void set_sample_rate(double sr) override;
        void set_volume(float vol) override;
        void set_pitch(float freq) override;
        void process(float* l, float* r, size_t nframes) override;

        void add_sample(const std::string& name, std::shared_ptr<disgrace_ns::SampleData> data);
        void remove_sample(size_t index);
        void move_sample(size_t from, size_t to);
        void set_sample_name(size_t index, const std::string& name);
        void convert_sample_format(size_t index, SampleFormatAction action);
        size_t sample_count() const { return m_samples.size(); }
        const SampleEntry& get_sample(size_t index) const { return m_samples[index]; }
        SampleEntry& get_sample(size_t index) { return m_samples[index]; }
        void update_sample_data(size_t index, std::shared_ptr<disgrace_ns::SampleData> data);

        void set_selected_sample(size_t index) { m_selected_sample_index = index; }
        size_t selected_sample() const { return m_selected_sample_index; }

        void push_undo(size_t index);
        void undo(size_t index);
        void redo(size_t index);
        bool can_undo(size_t index) const;
        bool can_redo(size_t index) const;

        double voice_position() const;

    protected:
        ::std::unique_ptr<disgrace_ns::Voice> create_voice() override;

    private:
        struct UndoState {
            std::vector<std::shared_ptr<disgrace_ns::SampleData>> undo_stack;
            std::vector<std::shared_ptr<disgrace_ns::SampleData>> redo_stack;
        };
        std::map<size_t, UndoState> m_undo_states;
        std::vector<SampleEntry> m_samples;
        size_t m_selected_sample_index = 0;
        double m_engine_rate;
    };

} // namespace disgrace_ns
