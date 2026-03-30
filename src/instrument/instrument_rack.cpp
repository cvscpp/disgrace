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

#include "instrument_rack.h"
#include "instrument.h" // Added for disgrace_ns::Instrument definition

namespace disgrace_ns
{

    size_t disgrace_ns::InstrumentRack::add(
        ::std::unique_ptr<disgrace_ns::Instrument> inst)
    {
        m_instruments.push_back(::std::move(inst));
        return m_instruments.size() - 1;
    }

    disgrace_ns::Instrument* disgrace_ns::InstrumentRack::get(size_t index)
    {
        if (index >= m_instruments.size())
            return nullptr;

        return m_instruments[index].get();
    }

    size_t disgrace_ns::InstrumentRack::size() const
    {
        return m_instruments.size();
    }

} // namespace disgrace_ns
