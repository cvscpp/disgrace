#pragma once
#include <vector>
#include <memory>
#include "command.h" // Add this line
#include "../edit/edit_command.h" // Add this line

namespace disgrace_ns
{

    class UndoStack
    {
    public:
        void execute(::std::unique_ptr<disgrace_ns::Command> cmd);
        void undo();
        void redo();
        void execute_group(::std::vector<EditCommandPtr> cmds); // Add this line

    private:
        ::std::vector<::std::unique_ptr<disgrace_ns::Command>> m_undo;
        ::std::vector<::std::unique_ptr<disgrace_ns::Command>> m_redo;
    };

} // namespace disgrace_ns
