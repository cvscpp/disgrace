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

#include <string>
#include <vector>
#include <cstdint>

namespace disgrace_ns {

class Engine;

// Open-string note (MIDI number) per subtrack, indexed by tracker column.
// One column of the chosen track == one string of the instrument.
struct TabExportConfig {
    size_t track = 0;
    std::vector<int> open_strings; // size == column_count(track), one MIDI note per column
    bool is_bass = false;
};

class TabExporter {
public:
    static bool export_track(const Engine& engine, const TabExportConfig& config, const std::string& path);
    static std::string generate_tab(const Engine& engine, const TabExportConfig& config);

    // "C-", "C#", ... plus display octave (MIDI 60 = "C-4"), matching the
    // tracker's note naming. Used for tuning labels and the dialog.
    static std::string note_name(int midi);

private:
    static const int kMaxFret = 24;
};

} // namespace disgrace_ns
