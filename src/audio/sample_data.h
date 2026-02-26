#pragma once
#include <vector>

namespace disgrace_ns
{

    struct SampleData
    {
        ::std::vector<float> left;
        ::std::vector<float> right;
        int sample_rate = 44100;
    };

} // namespace disgrace_ns
