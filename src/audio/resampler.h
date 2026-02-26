#pragma once
#include <vector>

namespace disgrace_ns
{

    class Resampler
    {
    public:
        static bool process(const ::std::vector<float>& input,
                            ::std::vector<float>& output,
                            double ratio);
    };

} // namespace disgrace_ns
