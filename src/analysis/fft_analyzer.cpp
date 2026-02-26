#include "fft_analyzer.h"
#include <fftw3.h> // Changed from kissfft/kiss_fft.h
#include <cmath>
#include <numeric> // For std::iota if needed, not strictly for FFTW, but good for array init

namespace disgrace_ns
{

    FFTAnalyzer::FFTAnalyzer(size_t size)
    : m_size(size),
      m_magnitudes(size / 2 + 1, 0.0f) // Size for real FFT magnitudes is N/2 + 1
    {
        // Allocate input and output arrays for FFTW
        m_in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_size);
        m_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_size);

        // Create a 1D FFTW plan for complex-to-complex DFT
        // FFTW_ESTIMATE is a flag that tells FFTW to choose an algorithm without
        // running any benchmarks. This is good for one-time plans.
        m_plan = fftw_plan_dft_1d(m_size, m_in, m_out, FFTW_FORWARD, FFTW_ESTIMATE);
    }

    FFTAnalyzer::~FFTAnalyzer()
    {
        fftw_destroy_plan(m_plan);
        fftw_free(m_in);
        fftw_free(m_out);
    }

    void FFTAnalyzer::process(const float* input)
    {
        // Copy float input to complex input, setting imaginary parts to zero
        for (size_t i = 0; i < m_size; ++i)
        {
            m_in[i][0] = input[i]; // Real part
            m_in[i][1] = 0.0;      // Imaginary part
        }

        // Execute the plan
        fftw_execute(m_plan);

        // Calculate magnitudes (sqrt(real^2 + imag^2))
        // For a real input, the output is symmetric. We only need the first N/2 + 1 points.
        for (size_t i = 0; i <= m_size / 2; ++i)
        {
            double real = m_out[i][0];
            double imag = m_out[i][1];
            m_magnitudes[i] = static_cast<float>(::std::sqrt(real * real + imag * imag));
        }
    }

    const ::std::vector<float>& FFTAnalyzer::magnitudes() const
    {
        return m_magnitudes;
    }

} // namespace disgrace_ns
