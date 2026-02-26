#pragma once
#include <array>
#include <cstddef> // Added for size_t

namespace disgrace_ns
{

    constexpr size_t MAX_AUTOMATION_POINTS = 512;

    struct AutomationPoint
    {
        size_t row;
        float value;
    };

    class AutomationLane
    {
    public:
        void add_point(size_t row, float value);
        float value_at(size_t row) const;

    private:
        ::std::array<disgrace_ns::AutomationPoint,
        MAX_AUTOMATION_POINTS> m_points{};
        size_t m_count = 0;
    };

} // namespace disgrace_ns
