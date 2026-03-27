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