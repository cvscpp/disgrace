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

namespace disgrace_ns {

class LV2Instrument : public Instrument {
public:
    LV2Instrument(double sample_rate) {}
    void note_on(uint8_t note, uint8_t velocity, size_t column_index = 0, size_t offset_samples = 0, uint8_t sample_index = 0) override {}
    void note_off(size_t column_index = 0) override {}
    void panic() override {}
    void set_volume(float vol) override {}
    void set_pitch(float freq) override {}
    void process(float* l, float* r, size_t nframes) override {
        for(size_t i=0; i<nframes; ++i) { l[i]=0; r[i]=0; }
    }
protected:
    std::unique_ptr<Voice> create_voice() override { return nullptr; }
};

} // namespace disgrace_ns
