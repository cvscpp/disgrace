#include "sample_buffer.h"
#include <algorithm>
#include <cmath>

namespace disgrace_ns
{

    void disgrace_ns::SampleBuffer::normalize()
    {
        float max_amp = 0.0f;

        for (float v : left)
            max_amp = ::std::max(max_amp, ::std::fabs(v));

        for (float v : right)
            max_amp = ::std::max(max_amp, ::std::fabs(v));

        if (max_amp <= 0.000001f)
            return;

        float gain = 1.0f / max_amp;
        apply_gain(gain);
    }

    void disgrace_ns::SampleBuffer::crop(size_t start, size_t end)
    {
        if (start >= end)
            return;

        if (end > left.size())
            end = left.size();

        left = ::std::vector<float>(left.begin() + start,
                                  left.begin() + end);

        if (!right.empty())
        {
            right = ::std::vector<float>(right.begin() + start,
                                       right.begin() + end);
        }
    }

    void disgrace_ns::SampleBuffer::apply_gain(float g)
    {
        for (auto& v : left)
            v *= g;

        for (auto& v : right)
            v *= g;
    }

} // namespace disgrace_ns
