#pragma once
#include <vector>
#include <memory>

namespace dg
{

    class UndoStack
    {
    public:
        void execute(std::unique_ptr<Command> cmd);
        void undo();
        void redo();

    private:
        std::vector<std::unique_ptr<Command>> m_undo;
        std::vector<std::unique_ptr<Command>> m_redo;
    };

}
