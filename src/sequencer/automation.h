#pragma once
#include <array>

namespace dg
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
        std::array<AutomationPoint,
        MAX_AUTOMATION_POINTS> m_points{};
        size_t m_count = 0;
    };

}
