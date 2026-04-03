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

#include "edit_command.h"
#include <vector>

namespace disgrace_ns {

class Track;
class Engine;

class TrackCutCommand : public EditCommand {
public:
    TrackCutCommand(Track& track, Engine& engine, size_t start_sample, size_t end_sample);
    ~TrackCutCommand() override = default;

    void apply() override;
    void undo() override;

    const char* name() const { return "Track Cut"; }

private:
    Track& m_track;
    Engine& m_engine;
    size_t m_start_sample;
    size_t m_end_sample;
    std::vector<float> m_saved_left;
    std::vector<float> m_saved_right;
    bool m_has_stereo = false;
};

} // namespace disgrace_ns
