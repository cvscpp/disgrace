#include <memory>
#include <utility>
#include <iterator> // Add this for std::make_move_iterator
#include "undo_stack.h" // Assuming this is needed for class definition

namespace disgrace_ns
{

void UndoStack::execute(::std::unique_ptr<disgrace_ns::Command> cmd)
{
    cmd->execute();
    m_undo.push_back(::std::move(cmd));
    m_redo.clear();
}

void UndoStack::undo()
{
    if (m_undo.empty())
        return;

    auto cmd = ::std::move(m_undo.back());
    m_undo.pop_back();

    cmd->undo();
    m_redo.push_back(::std::move(cmd));
}

void UndoStack::redo()
{
    if (m_redo.empty())
        return;

    auto cmd = ::std::move(m_redo.back());
    m_redo.pop_back();

    cmd->execute();
    m_undo.push_back(::std::move(cmd));
}

void UndoStack::execute_group(::std::vector<EditCommandPtr> cmds)
{
    for (auto& cmd : cmds)
    {
        cmd->execute();
    }
    for (auto& cmd : cmds) // Loop to cast and move
    {
        m_undo.push_back(static_cast<::std::unique_ptr<disgrace_ns::Command>>(::std::move(cmd)));
    }
    m_redo.clear();
}

} // namespace disgrace_ns
