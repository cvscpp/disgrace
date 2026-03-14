#include "spectral_view.h"
#include "../core/engine.h"
#include <FL/fl_draw.H>
#include <cmath>
#include <algorithm>

namespace disgrace_ns {

SpectralView::SpectralView(int x, int y, int w, int h, Engine& engine)
    : Fl_Box(x, y, w, h), m_engine(engine) {
    box(FL_FLAT_BOX);
    color(FL_BLACK);
    m_analyzer = std::make_unique<FFTAnalyzer>(m_fft_size);
    m_history.resize(m_fft_size / 2 + 1, 0.0f);
    m_fft_input.resize(m_fft_size, 0.0f);
}

SpectralView::~SpectralView() {}

void SpectralView::update() {
    float val;
    bool has_data = false;
    while (m_engine.m_spectral_rb.pop(val)) {
        std::copy(m_fft_input.begin() + 1, m_fft_input.end(), m_fft_input.begin());
        m_fft_input[m_fft_size - 1] = val;
        has_data = true;
    }

    if (has_data) {
        m_analyzer->process(m_fft_input.data());
        const auto& mags = m_analyzer->magnitudes();
        for (size_t i = 0; i < mags.size(); ++i) {
            m_history[i] = m_history[i] * 0.7f + mags[i] * 0.3f; // Simple smoothing
        }
        redraw();
    }
}

void SpectralView::draw() {
    Fl_Box::draw();

    int bx = x() + 2;
    int by = y() + 2;
    int bw = w() - 4;
    int bh = h() - 4;

    fl_color((Fl_Color)m_engine.m_tracker_bg);
    fl_rectf(bx, by, bw, bh);

    if (m_history.empty()) return;

    int num_bars = 40;
    int bar_w = bw / num_bars;
    if (bar_w < 1) bar_w = 1;

    for (int i = 0; i < num_bars; ++i) {
        // Logarithmic X axis mapping for bars
        float freq_norm_start = (float)i / num_bars;
        float freq_norm_end = (float)(i + 1) / num_bars;
        
        float log_freq_start = std::pow(10.0f, freq_norm_start * 3.0f) / 1000.0f;
        float log_freq_end = std::pow(10.0f, freq_norm_end * 3.0f) / 1000.0f;
        
        size_t bin_start = (size_t)(log_freq_start * (m_history.size() - 1));
        size_t bin_end = (size_t)(log_freq_end * (m_history.size() - 1));
        if (bin_end <= bin_start) bin_end = bin_start + 1;
        if (bin_end > m_history.size()) bin_end = m_history.size();

        float max_mag = 0.0f;
        for (size_t b = bin_start; b < bin_end; ++b) {
            if (m_history[b] > max_mag) max_mag = m_history[b];
        }

        float db = 20.0f * std::log10(max_mag + 1e-6f);
        float y_norm = (db + 60.0f) / 60.0f;
        if (y_norm < 0.0f) y_norm = 0.0f;
        if (y_norm > 1.0f) y_norm = 1.0f;

        int bar_h = (int)(y_norm * bh);
        int cur_x = bx + i * bar_w;
        int cur_y = by + bh - bar_h;

        // Draw bar with segments or gradient
        for (int j = 0; j < bar_h; ++j) {
            float h_norm = (float)j / bh;
            if (h_norm < 0.5f) fl_color(FL_GREEN);
            else if (h_norm < 0.8f) fl_color(FL_YELLOW);
            else fl_color(FL_RED);
            fl_line(cur_x, by + bh - j, cur_x + bar_w - 2, by + bh - j);
        }
    }
    
    // Draw grid
    fl_color((Fl_Color)m_engine.m_tracker_lpb_highlight);
    for (int db = -40; db <= 0; db += 20) {
        float y_norm = (db + 60.0f) / 60.0f;
        int gy = by + bh - (int)(y_norm * bh);
        fl_line(bx, gy, bx + bw, gy);
    }
}

} // namespace disgrace_ns
