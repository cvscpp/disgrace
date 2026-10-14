#pragma once

#include <wx/wxprec.h>
#include <wx/panel.h>

namespace disgrace_ns {

class Engine;

class VUMeter : public wxPanel {
public:
    VUMeter(wxWindow* parent, wxWindowID id, Engine& engine, bool horizontal = false);

    void level(float l);
    float level() const { return m_level; }

    void OnPaint(wxPaintEvent& event);

private:
    void paint_meter(wxDC& dc);
    Engine& m_engine;
    float m_level = 0.0f;
    float m_peak_hold = 0.0f;
    int m_peak_timer = 0;
    bool m_horizontal = false;
    int m_last_level_px = -1;
    int m_last_peak_px = -1;

    DECLARE_EVENT_TABLE()
};

} // namespace disgrace_ns
