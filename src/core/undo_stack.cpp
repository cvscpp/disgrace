void UndoStack::execute(std::unique_ptr<Command> cmd)
{
    cmd->execute();
    m_undo.push_back(std::move(cmd));
    m_redo.clear();
}

void UndoStack::undo()
{
    if (m_undo.empty())
        return;

    auto cmd = std::move(m_undo.back());
    m_undo.pop_back();

    cmd->undo();
    m_redo.push_back(std::move(cmd));
}

void UndoStack::redo()
{
    if (m_redo.empty())
        return;

    auto cmd = std::move(m_redo.back());
    m_redo.pop_back();

    cmd->execute();
    m_undo.push_back(std::move(cmd));
}
