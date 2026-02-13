#pragma once
#include "edit_command.h"
#include <vector>

namespace dg
{

    class UndoStack
    {
    public:
        void execute(EditCommandPtr cmd)
        {
            cmd->apply();

            m_undo.push_back(std::move(cmd));
            m_redo.clear();
        }

        void undo()
        {
            if (m_undo.empty())
                return;

            auto cmd =
            std::move(m_undo.back());

            m_undo.pop_back();

            cmd->undo();
            m_redo.push_back(std::move(cmd));
        }

        void redo()
        {
            if (m_redo.empty())
                return;

            auto cmd =
            std::move(m_redo.back());

            m_redo.pop_back();

            cmd->apply();
            m_undo.push_back(std::move(cmd));
        }

        void execute_group(
            std::vector<EditCommandPtr> cmds)
        {
            for (auto& c : cmds)
                c->apply();

            m_undo.push_back(
                std::make_unique<GroupCommand>(
                    std::move(cmds)));

            m_redo.clear();
        }


        bool can_undo() const { return !m_undo.empty(); }
        bool can_redo() const { return !m_redo.empty(); }

    private:
        std::vector<EditCommandPtr> m_undo;
        std::vector<EditCommandPtr> m_redo;
    };

    class GroupCommand : public EditCommand
    {
    public:
        GroupCommand(std::vector<EditCommandPtr> cmds)
        : m_cmds(std::move(cmds)) {}

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
        std::vector<EditCommandPtr> m_cmds;
    };

}
