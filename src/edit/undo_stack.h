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

namespace disgrace_ns
{

    class GroupCommand : public disgrace_ns::EditCommand
    {
    public:
        GroupCommand(::std::vector<disgrace_ns::EditCommandPtr> cmds)
        : m_cmds(::std::move(cmds)) {}

        void apply() override
        {
            for (auto& c : m_cmds)
                c->apply();
        }

        void undo() override
        {
            for (auto it = m_cmds.rbegin();
                 it != m_cmds.rend(); ++it)
                 (*it)->undo();
        }

    private:
        ::std::vector<disgrace_ns::EditCommandPtr> m_cmds;
    };

    class UndoStack
    {

    };

} // namespace disgrace_ns
