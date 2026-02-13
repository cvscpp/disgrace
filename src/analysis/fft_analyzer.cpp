#include "fft_analyzer.h"
#include <kissfft/kiss_fft.h>
#include <cmath>

namespace dg
{

    FFTAnalyzer::FFTAnalyzer(size_t size)
    : m_size(size),
    m_output(size / 2, 0.0f)
    {
    }

    void FFTAnalyzer::process(const float* input)
    {
        kiss_fft_cfg cfg =
        kiss_fft_alloc(m_size, 0, nullptr, nullptr);

        std::vector<kiss_fft_cpx> in(m_size);
        std::vector<kiss_fft_cpx> out(m_size);

        for (size_t i = 0; i < m_size; ++i)
        {
            in[i].r = input[i];
            in[i].i = 0.0f;
        }

        kiss_fft(cfg, in.data(), out.data());

        for (size_t i = 0; i < m_size / 2; ++i)
        {
            float real = out[i].r;
            float imag = out[i].i;

            m_output[i] =
            std::sqrt(real * real + imag * imag);
        }

        free(cfg);
    }

    const std::vector<float>& FFTAnalyzer::magnitudes() const
    {
        return m_output;
    }

}
