#pragma once
#include <vector>

namespace disgrace_ns
{

    struct SampleData
    {
        ::std::vector<float> left;
        ::std::vector<float> right;
        int sample_rate = 44100;

        void to_mono_l() { right.clear(); }
        void to_mono_r() { if (!right.empty()) left = right; right.clear(); }
        void to_mono_mix() {
            if (right.empty()) return;
            for (size_t i = 0; i < left.size(); ++i) {
                left[i] = (left[i] + right[i]) * 0.5f;
            }
            right.clear();
        }
        void to_stereo() {
            if (!right.empty()) return;
            right = left;
        }
    };

} // namespace disgrace_ns
