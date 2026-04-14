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

#include "../core/engine.h" // ADDED
#include <cmath> // Added for powf
#include "instrument.h"

namespace disgrace_ns
{

    disgrace_ns::Voice* disgrace_ns::Instrument::allocate_voice(size_t column_index)
    {
        // find inactive voice
        for (auto& v : m_voices)
        {
            if (v && !v->active()) {
                v->set_column(column_index);
                return v.get();
            }
        }

        // voice stealing: steal oldest (voice 0)
        if (m_voices[0]) {
            m_voices[0]->set_column(column_index);
            return m_voices[0].get();
        }
        return nullptr;
    }

} // namespace disgrace_ns
