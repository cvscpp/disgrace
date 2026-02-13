#pragma once
#include <vector>

namespace dg
{

    class Resampler
    {
    public:
        static bool process(const std::vector<float>& input,
                            std::vector<float>& output,
                            double ratio);
    };

}
