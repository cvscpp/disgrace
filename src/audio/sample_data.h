#pragma once
#include <vector>

namespace dg
{

    struct SampleData
    {
        std::vector<float> left;
        std::vector<float> right;
        int sample_rate = 44100;
    };

}
