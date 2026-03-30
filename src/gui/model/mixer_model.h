#pragma once

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

#include <vector>

namespace disgrace_ns
{

    struct MixerChannelState
    {
        float volume;
        float pan;
        float peak_l;
        float peak_r;
    };

    class MixerModel
    {
    public:
        void update_from_engine(const ::std::vector<disgrace_ns::MixerChannelState>& state);

        const ::std::vector<disgrace_ns::MixerChannelState>& channels() const;

    private:
        ::std::vector<disgrace_ns::MixerChannelState> m_channels;
    };

} // namespace disgrace_ns
