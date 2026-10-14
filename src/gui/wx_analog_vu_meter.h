#pragma once

#include <wx/wxprec.h>
#include <wx/panel.h>
#include <wx/bitmap.h>

namespace disgrace_ns {

class Engine;

class AnalogVUMeter : public wxPanel {
public:
    AnalogVUMeter(wxWindow* parent, wxWindowID id, Engine& engine);

    void level(float l);
    float level() const { return m_level; }

    void OnPaint(wxPaintEvent& event);

private:
    void paint_face(wxDC& dc, int w, int h);
    void draw_needle(wxDC& dc, int w, int h);

    Engine& m_engine;
    float m_level = 0.0f;
    float m_smooth_level = 0.0f;
    int m_last_bucket = -1;
    wxBitmap m_face;

    wxDECLARE_EVENT_TABLE();
};

} // namespace disgrace_ns
