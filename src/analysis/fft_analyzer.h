#pragma once
#include <vector>

namespace dg
{

    class FFTAnalyzer
    {
    public:
        FFTAnalyzer(size_t size);

        void process(const float* input);

        const std::vector<float>& magnitudes() const;

    private:
        size_t m_size;
        std::vector<float> m_output;
    };

}
