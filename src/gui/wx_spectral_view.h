#pragma once

#include <wx/wxprec.h>
#include <wx/panel.h>
#include <memory>
#include <vector>
#include <memory>
#include "../analysis/fft_analyzer.h"

namespace disgrace_ns {

class Engine;

class SpectralView : public wxPanel {
public:
    SpectralView(wxWindow* parent, wxWindowID id, Engine& engine);
    ~SpectralView();

    void update();
    void OnPaint(wxPaintEvent& event);

private:
    Engine& m_engine;
    std::unique_ptr<FFTAnalyzer> m_analyzer;
    std::vector<float> m_fft_input;     // circular buffer
    std::vector<float> m_windowed;      // pre-allocated FFT work buffer
    std::vector<float> m_window_coeffs; // pre-computed Hanning window
    std::vector<float> m_magnitudes;
    size_t m_fft_size  = 2048;
    size_t m_write_pos = 0;    // next write slot in circular buffer
    size_t m_new_samples = 0;  // samples accumulated since last FFT run
    static constexpr size_t kHopSize = 512; // FFT update interval (samples)

    wxDECLARE_EVENT_TABLE();
};

} // namespace disgrace_ns
