#pragma once
#include <memory>

namespace dg
{

    class EditCommand
    {
    public:
        virtual ~EditCommand() = default;

        virtual void apply() = 0;
        virtual void undo()  = 0;
    };

    using EditCommandPtr =
    std::unique_ptr<EditCommand>;

}
