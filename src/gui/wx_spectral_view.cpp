#include "wx_spectral_view.h"
#include "theme.h"
#include "../core/engine.h"
#include <wx/dcclient.h>
#include <wx/dcbuffer.h>
#include <cmath>
#include <algorithm>

namespace disgrace_ns {

wxBEGIN_EVENT_TABLE(SpectralView, wxPanel)
    EVT_PAINT(SpectralView::OnPaint)
wxEND_EVENT_TABLE()

SpectralView::SpectralView(wxWindow* parent, wxWindowID id, Engine& engine)
    : wxPanel(parent, id), m_engine(engine)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetDoubleBuffered(true);
    m_fft_size = 2048;
    m_analyzer = std::make_unique<FFTAnalyzer>(m_fft_size);
    m_fft_input.resize(m_fft_size, 0.0f);
    m_windowed.resize(m_fft_size, 0.0f);
    m_magnitudes.resize(m_fft_size / 2 + 1, 0.0f);

    // Pre-compute Hanning window coefficients once.
    m_window_coeffs.resize(m_fft_size);
    for (size_t i = 0; i < m_fft_size; ++i)
        m_window_coeffs[i] = 0.5f * (1.0f - cosf(2.0f * 3.14159265f * i / (m_fft_size - 1)));
}

SpectralView::~SpectralView() {}

void SpectralView::update() {
    float val;
    while (m_engine.m_spectral_rb.pop(val)) {
        m_fft_input[m_write_pos & (m_fft_size - 1)] = val;
        ++m_write_pos;
        ++m_new_samples;
    }

    if (m_new_samples < kHopSize) return;
    m_new_samples = 0;

    for (size_t i = 0; i < m_fft_size; ++i) {
        size_t idx = (m_write_pos - m_fft_size + i) & (m_fft_size - 1);
        m_windowed[i] = m_fft_input[idx] * m_window_coeffs[i];
    }

    m_analyzer->process(m_windowed.data());
    const auto& new_mags = m_analyzer->magnitudes();

    const float norm_factor = 1.0f / (float)(m_fft_size / 2);
    float alpha = 0.25f;
    for (size_t i = 0; i < m_magnitudes.size(); ++i)
        m_magnitudes[i] = alpha * (new_mags[i] * norm_factor) + (1.0f - alpha) * m_magnitudes[i];

    Refresh(false);
}

// Map normalized level [0,1] to a smooth color gradient:
// 0.0 = dark blue, 0.2 = cyan, 0.4 = green, 0.6 = yellow, 0.8 = orange, 1.0 = red
static wxColour gradient_color(float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    int r, g, b;
    if (t < 0.2f) {
        float s = t / 0.2f;
        r = 0; g = (int)(s * 200); b = (int)(100 + s * 155);
    } else if (t < 0.4f) {
        float s = (t - 0.2f) / 0.2f;
        r = 0; g = (int)(200 + s * 55); b = (int)(255 - s * 255);
    } else if (t < 0.6f) {
        float s = (t - 0.4f) / 0.2f;
        r = (int)(s * 255); g = 255; b = 0;
    } else if (t < 0.8f) {
        float s = (t - 0.6f) / 0.2f;
        r = 255; g = (int)(255 - s * 128); b = 0;
    } else {
        float s = (t - 0.8f) / 0.2f;
        r = 255; g = (int)(127 - s * 127); b = 0;
    }
    return wxColour(r, g, b);
}

void SpectralView::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    wxSize size = GetClientSize();
    int w = size.GetWidth();
    int h = size.GetHeight();

    dc.SetBrush(wxBrush(ThemeManager::toWxColour(m_engine.m_tracker_bg)));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(0, 0, w, h);

    if (m_magnitudes.empty()) return;

    float sample_rate = (float)m_engine.sample_rate();
    float min_freq = 20.0f;
    float max_freq = std::min(20000.0f, sample_rate / 2.0f);
    float log_min = log10f(min_freq);
    float log_max = log10f(max_freq);
    float db_min = -60.0f;
    float db_max = 0.0f;

    int num_bars = 80;
    int bar_w = std::max(1, (w - 40) / num_bars);  // leave margin for dB labels
    int gap = 1;
    int left_margin = 40;
    int bottom_margin = 16;
    int plot_h = h - bottom_margin;
    int plot_w = w - left_margin;

    // Horizontal grid lines (dB) and labels
    dc.SetFont(wxFont(7, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.SetTextForeground(wxColour(140, 140, 140));
    float db_ticks[] = { -60, -50, -40, -30, -20, -10, 0 };
    for (float db : db_ticks) {
        float norm_y = (db - db_min) / (db_max - db_min);
        int gy = plot_h - (int)(norm_y * plot_h);
        dc.SetPen(wxPen(wxColour(50, 50, 50), 1, wxPENSTYLE_DOT));
        dc.DrawLine(left_margin, gy, w, gy);
        wxString label;
        if (db == 0) label = "0 dB";
        else label.Printf("%d", (int)db);
        wxSize ts = dc.GetTextExtent(label);
        dc.DrawText(label, 2, gy - ts.y / 2);
    }

    // Vertical grid lines (frequency) and labels
    float freq_marks[] = { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    for (float f : freq_marks) {
        if (f < min_freq || f > max_freq) continue;
        float log_f = log10f(f);
        int gx = left_margin + (int)(((log_f - log_min) / (log_max - log_min)) * plot_w);
        dc.SetPen(wxPen(wxColour(50, 50, 50), 1, wxPENSTYLE_DOT));
        dc.DrawLine(gx, 0, gx, plot_h);
        wxString label;
        if (f >= 1000) label.Printf("%.0fk", f / 1000.0f);
        else label.Printf("%.0f", f);
        dc.SetTextForeground(wxColour(120, 120, 120));
        wxSize ts = dc.GetTextExtent(label);
        dc.DrawText(label, gx - ts.x / 2, plot_h + 2);
    }

    // Draw bars with smooth gradient
    for (int i = 0; i < num_bars; ++i) {
        float f_start = powf(10.0f, log_min + ((float)i / num_bars) * (log_max - log_min));
        float f_end = powf(10.0f, log_min + ((float)(i + 1) / num_bars) * (log_max - log_min));

        float bin_start = f_start * (float)m_fft_size / sample_rate;
        float bin_end = f_end * (float)m_fft_size / sample_rate;

        float max_mag = 0.0f;
        size_t start_idx = (size_t)bin_start;
        size_t end_idx = (size_t)ceilf(bin_end);

        for (size_t idx = start_idx; idx < end_idx && idx < m_magnitudes.size(); ++idx)
            max_mag = std::max(max_mag, m_magnitudes[idx]);

        float db = 20.0f * log10f(max_mag + 1e-6f);
        float norm_y = (db - db_min) / (db_max - db_min);
        norm_y = std::max(0.0f, std::min(1.0f, norm_y));

        int bar_h = (int)(norm_y * plot_h);
        int x = left_margin + i * (bar_w + gap);

        // Per-pixel vertical gradient
        for (int py = 0; py < bar_h; ++py) {
            float t = (float)py / (float)plot_h;
            wxColour c = gradient_color(t);
            dc.SetPen(wxPen(c));
            dc.DrawLine(x, plot_h - py, x + bar_w - 1, plot_h - py);
        }

        // Peak hold
        if ((int)m_peak_hold.size() <= i) {
            m_peak_hold.resize(i + 1, 0.0f);
            m_peak_timer.resize(i + 1, 0);
        }
        if (norm_y > m_peak_hold[i]) {
            m_peak_hold[i] = norm_y;
            m_peak_timer[i] = 30;  // ~500ms hold at 60fps
        } else if (m_peak_timer[i] > 0) {
            --m_peak_timer[i];
        } else {
            m_peak_hold[i] -= 0.005f;  // slow decay
            if (m_peak_hold[i] < 0.0f) m_peak_hold[i] = 0.0f;
        }

        if (m_peak_hold[i] > 0.01f) {
            int peak_y = plot_h - (int)(m_peak_hold[i] * plot_h);
            dc.SetPen(wxPen(wxColour(255, 255, 255, 200), 1));
            dc.DrawLine(x, peak_y, x + bar_w - 1, peak_y);
        }
    }
}

} // namespace disgrace_ns
