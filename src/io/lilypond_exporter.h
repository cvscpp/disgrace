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

class LilyPondExporter {
public:
    static bool export_project(const Engine& engine, const std::string& path);
    static std::string generate_ly_source(const Engine& engine, int track_index = -1);

private:
    static std::string midi_to_lily(uint8_t note);
    static std::string get_clef_string(int notation_type);
};

} // namespace disgrace_ns
