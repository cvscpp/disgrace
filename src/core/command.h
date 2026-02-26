#pragma once

namespace disgrace_ns
{

    class Command
    {
    public:
        virtual ~Command() = default;

        virtual void execute() = 0;
        virtual void undo() = 0;
    };

} // namespace disgrace_ns
