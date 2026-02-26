#pragma once
#include <vector>
#include <cstdint> // Add this line

namespace disgrace_ns
{

    class SampleBuffer
    {
    public:
        ::std::vector<float> left;
        ::std::vector<float> right;
        uint32_t sample_rate = 44100;

        void normalize();
        void crop(size_t start, size_t end);
        void apply_gain(float g);
    };

} // namespace disgrace_ns
