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

#include <FL/Fl_Box.H>
#include <vector>
#include <memory>
#include "../analysis/fft_analyzer.h"

namespace disgrace_ns {

class Engine;

class SpectralView : public Fl_Box {
public:
    SpectralView(int x, int y, int w, int h, Engine& engine);
    ~SpectralView();

    void update();
    void draw() override;

private:
    Engine& m_engine;
    std::unique_ptr<FFTAnalyzer> m_analyzer;
    std::vector<float> m_history;
    std::vector<float> m_fft_input;
    size_t m_fft_size = 1024;
};

} // namespace disgrace_ns
