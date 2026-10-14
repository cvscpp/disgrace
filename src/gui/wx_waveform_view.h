#pragma once

#include <wx/wxprec.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <memory>

#include "../audio/sample_data.h"

namespace disgrace_ns {

class Engine;

enum class ChannelMode { Both, Left, Right };

class WaveformView : public wxPanel {
public:
    WaveformView(wxWindow* parent, wxWindowID id, Engine& engine);

    void set_sample(std::shared_ptr<SampleData> s);
    void set_sample(std::shared_ptr<SampleData> s, size_t visible_length);
    void set_color(unsigned int c) { m_color = c; Refresh(); }

    void zoom_in();
    void zoom_out();
    void view_all();
    void view_selection();

    void OnPaint(wxPaintEvent& event);
    void paint_wave(wxDC& dc);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseDrag(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);

    size_t selection_start() const { return m_sel_start < m_sel_end ? m_sel_start : m_sel_end; }
    size_t selection_end()   const { return m_sel_start < m_sel_end ? m_sel_end : m_sel_start; }
    // Which stereo channel(s) the current selection applies to (CH_LEFT/CH_RIGHT/CH_BOTH).
    uint8_t selection_channels() const { return m_sel_channels; }
    void set_channel_mode(ChannelMode mode) { m_mode = mode; Refresh(false); }
    void set_playback_pos(int64_t pos) {
        int64_t next = pos < 0 ? -1 : pos;
        if (next == m_playback_pos) return;
        m_playback_pos = next;
        if (IsShownOnScreen()) Refresh(false);
    }

private:
    void get_view_range(size_t& start, size_t& end);
    size_t sample_length() const;
    bool is_stereo() const;
    // Returns the channel (CH_LEFT/CH_RIGHT/CH_BOTH) a vertical mouse position
    // falls into, given the current display mode.
    uint8_t channel_at_y(int y) const;
    // Returns pixel x for a sample position (or -1 if outside view).
    int sample_to_x(size_t pos, size_t view_start, size_t view_end, int width) const;

    Engine& m_engine;
    std::shared_ptr<SampleData> m_sample;
    size_t m_visible_length = 0;
    unsigned int m_color = 0x40FF4000;

    size_t m_sel_start = 0;
    size_t m_sel_end   = 0;
    // Channels covered by the current selection, and the channel where the
    // current drag started (anchors the vertical channel span).
    uint8_t m_sel_channels  = CH_BOTH;
    uint8_t m_sel_anchor_ch = CH_LEFT;

    double m_zoom   = 1.0;
    size_t m_offset = 0;

    ChannelMode m_mode = ChannelMode::Both;

    int64_t m_playback_pos = -1;
    bool m_painting_cache = false;
    wxBitmap m_wave_cache;
    // Identity of the cached wave (everything except the playback cursor).
    int m_cache_w = 0;
    int m_cache_h = 0;
    int m_cache_mode = -1;
    unsigned m_cache_color = 0;
    unsigned m_cache_bg = 0;
    double m_cache_zoom = -1;
    size_t m_cache_offset = (size_t)-1;
    size_t m_cache_length = (size_t)-1;
    size_t m_cache_visible = (size_t)-1;
    size_t m_cache_sel_s = (size_t)-1;
    size_t m_cache_sel_e = (size_t)-1;
    uint8_t m_cache_sel_ch = 0;
    const void* m_cache_sample = nullptr;
    float m_cache_probe = 0;

    enum class DragMode { None, NewSel, DragStart, DragEnd };
    DragMode m_drag_mode = DragMode::None;

    static constexpr int EDGE_THRESH = 10; // pixels

    wxDECLARE_EVENT_TABLE();
};

} // namespace disgrace_ns
