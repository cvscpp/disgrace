#pragma once
#include <vector>
#include <fftw3.h> // Added for FFTW3

namespace disgrace_ns
{

    class FFTAnalyzer
    {
    public:
        FFTAnalyzer(size_t size);
        ~FFTAnalyzer(); // Destructor to destroy FFTW plan

        void process(const float* input);

        const ::std::vector<float>& magnitudes() const;

    private:
        size_t m_size;
        ::std::vector<float> m_magnitudes; // Renamed from m_output to be more descriptive

        // FFTW3 members
        fftw_complex *m_in, *m_out; // Input and output buffers
        fftw_plan m_plan;           // FFTW plan
    };

} // namespace disgrace_ns
