#include "wx_waveform_view.h"
#include "theme.h"
#include "../core/engine.h"

#include <wx/dcclient.h>

namespace disgrace_ns {

wxBEGIN_EVENT_TABLE(WaveformView, wxPanel)
    EVT_PAINT(WaveformView::OnPaint)
    EVT_LEFT_DOWN(WaveformView::OnMouseDown)
    EVT_MOTION(WaveformView::OnMouseDrag)
    EVT_LEFT_UP(WaveformView::OnMouseUp)
    EVT_MOUSEWHEEL(WaveformView::OnMouseWheel)
wxEND_EVENT_TABLE()

WaveformView::WaveformView(wxWindow* parent, wxWindowID id, Engine& engine)
    : wxPanel(parent, id), m_engine(engine)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void WaveformView::set_sample(std::shared_ptr<SampleData> s) {
    m_sample = s;
    Refresh();
}

void WaveformView::get_view_range(size_t& start, size_t& end) {
    if (!m_sample || m_sample->left.empty()) {
        start = 0;
        end = 0;
        return;
    }
    size_t total = m_sample->left.size();
    start = m_offset;
    size_t visible = (size_t)(total / m_zoom);
    if (visible < 10) visible = 10; // Minimum 10 samples visible
    end = start + visible;
    if (end > total) {
        end = total;
        if (end > visible) start = end - visible;
        else start = 0;
        m_offset = start;
    }
}

void WaveformView::zoom_in() {
    m_zoom *= 1.5;
    // We should ideally zoom around the center, but keeping it simple for now
    Refresh();
}

void WaveformView::zoom_out() {
    m_zoom /= 1.5;
    if (m_zoom < 1.0) {
        m_zoom = 1.0;
        m_offset = 0;
    }
    Refresh();
}

void WaveformView::view_all() {
    m_zoom = 1.0;
    m_offset = 0;
    Refresh();
}

void WaveformView::view_selection() {
    if (m_sample && m_sel_start != m_sel_end) {
        size_t start = std::min(m_sel_start, m_sel_end);
        size_t end = std::max(m_sel_start, m_sel_end);
        size_t visible = end - start;
        if (visible > 0) {
            m_offset = start;
            m_zoom = (double)m_sample->left.size() / visible;
            Refresh();
        }
    }
}

void WaveformView::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    wxSize size = GetClientSize();

    dc.SetBrush(wxBrush(ThemeManager::toWxColour(m_engine.m_tracker_bg)));
    dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_bg)));
    dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());

    if (!m_sample || m_sample->left.empty()) return;

    int time_scale_h = 20;
    int wave_h = size.GetHeight() - time_scale_h;
    
    size_t start, end;
    get_view_range(start, end);
    if (end <= start) return;

    double sample_rate = m_sample->sample_rate;
    double view_duration = (double)(end - start) / sample_rate;
    
    // Intelligent time scale interval
    double interval = 1.0;
    if (view_duration < 0.1) interval = 0.01;
    else if (view_duration < 0.5) interval = 0.05;
    else if (view_duration < 2.0) interval = 0.2;
    else if (view_duration < 10.0) interval = 1.0;
    else if (view_duration < 30.0) interval = 5.0;
    else interval = 10.0;

    // Draw grid and time scale
    dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_lpb_highlight)));
    dc.DrawLine(0, time_scale_h - 1, size.GetWidth(), time_scale_h - 1);

    double first_tick = floor((double)start / sample_rate / interval) * interval;
    for (double t = first_tick; t <= (double)end / sample_rate + interval; t += interval) {
        int x = (int)((t * sample_rate - start) / (end - start) * size.GetWidth());
        if (x < 0 || x >= size.GetWidth()) continue;

        dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_lpb_highlight)));
        dc.DrawLine(x, 0, x, size.GetHeight());
        
        dc.SetTextForeground(ThemeManager::toWxColour(m_engine.m_tracker_text));
        dc.SetFont(wxFont(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
        dc.DrawText(wxString::Format("%.2fs", t), x + 2, 2);
    }

    bool is_stereo = !m_sample->right.empty();
    bool show_both = (m_mode == ChannelMode::Both) && is_stereo;
    double samples_per_pixel = (double)(end - start) / size.GetWidth();

    auto draw_channel = [&](const std::vector<float>& data, int y_off, int h, const wxString& label) {
        int mid_y = y_off + h / 2;
        dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_lpb_highlight)));
        dc.DrawLine(0, mid_y, size.GetWidth(), mid_y);
        
        dc.SetTextForeground(ThemeManager::toWxColour(m_engine.m_tracker_text));
        dc.SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        dc.DrawText(label, 2, y_off + 2);

        dc.SetPen(wxPen(ThemeManager::toWxColour(m_color)));

        for (int i = 0; i < size.GetWidth(); ++i) {
            size_t s = start + (size_t)(i * samples_per_pixel);
            size_t e = start + (size_t)((i + 1) * samples_per_pixel);
            if (e > data.size()) e = data.size();
            if (s >= data.size()) continue;

            float min_v = 1.0f, max_v = -1.0f;
            if (e > s) {
                for (size_t j = s; j < e; ++j) {
                    float v = data[j];
                    if (v < min_v) min_v = v;
                    if (v > max_v) max_v = v;
                }
            } else {
                min_v = max_v = data[s];
            }

            int y1 = mid_y + (int)(min_v * (h / 2 - 2));
            int y2 = mid_y + (int)(max_v * (h / 2 - 2));
            if (y1 == y2) {
                dc.DrawPoint(i, y1);
            } else {
                dc.DrawLine(i, y1, i, y2);
            }
        }
    };

    if (show_both) {
        draw_channel(m_sample->left, time_scale_h, wave_h / 2, "L");
        draw_channel(m_sample->right, time_scale_h + wave_h / 2, wave_h / 2, "R");
        dc.SetPen(wxPen(ThemeManager::toWxColour(m_engine.m_tracker_lpb_highlight)));
        dc.DrawLine(0, time_scale_h + wave_h / 2, size.GetWidth(), time_scale_h + wave_h / 2);
    } else if (m_mode == ChannelMode::Right && is_stereo) {
        draw_channel(m_sample->right, time_scale_h, wave_h, "R");
    } else {
        draw_channel(m_sample->left, time_scale_h, wave_h, is_stereo ? "L" : "M");
    }

    // Draw selection
    if (m_sel_start != m_sel_end) {
        size_t s = std::min(m_sel_start, m_sel_end);
        size_t e = std::max(m_sel_start, m_sel_end);

        if (e > start && s < end) {
            int x1 = (int)((double)(std::max(s, start) - start) / (end - start) * size.GetWidth());
            int x2 = (int)((double)(std::min(e, end) - start) / (end - start) * size.GetWidth());
            
            dc.SetBrush(wxBrush(wxColor(255, 255, 255, 64)));
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.DrawRectangle(x1, time_scale_h, x2 - x1, wave_h);
        }
    }
}

void WaveformView::OnMouseDown(wxMouseEvent& event) {
    SetFocus();
    int x = event.GetX();
    size_t start, end;
    get_view_range(start, end);
    if (end > start) {
        size_t current_pos = start + (size_t)((double)x / GetClientSize().GetWidth() * (end - start));
        
        if (!m_is_selecting) {
            // Start new selection
            m_sel_start = current_pos;
            m_sel_end = m_sel_start;
            m_is_selecting = true;
        } else {
            // Finish selection on second click
            m_sel_end = current_pos;
            m_is_selecting = false;
        }
    }
    Refresh();
}

void WaveformView::OnMouseDrag(wxMouseEvent& event) {
    if (m_is_selecting) {
        int x = event.GetX();
        size_t start, end;
        get_view_range(start, end);
        if (end > start) {
            m_sel_end = start + (size_t)((double)x / GetClientSize().GetWidth() * (end - start));
        }
        Refresh();
    }
}

void WaveformView::OnMouseUp(wxMouseEvent& event) {
    // If we were dragging and the button is released, we might want to finish selection.
    // But to support "click-move-click", we only finish if it was a significant drag.
    if (m_is_selecting) {
        if (m_sel_start != m_sel_end) {
            // If they dragged, consider it done.
            m_is_selecting = false;
        }
    }
}

void WaveformView::OnMouseWheel(wxMouseEvent& event) {
    if (event.ControlDown()) {
        if (event.GetWheelRotation() < 0) zoom_out();
        else zoom_in();
    }
}

} // namespace disgrace_ns
