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
#include <memory>
#include "../core/command.h" // ADD THIS LINE

namespace disgrace_ns
{

    class EditCommand : public disgrace_ns::Command // INHERIT FROM COMMAND
    {
    public:
        virtual ~EditCommand() = default;

        virtual void apply() = 0;
        virtual void undo() override = 0;
        void execute() override { apply(); } // IMPLEMENT EXECUTE
    };

    using EditCommandPtr =
    ::std::unique_ptr<disgrace_ns::EditCommand>;

} // namespace disgrace_ns