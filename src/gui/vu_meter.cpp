#include "vu_meter.h"
#include <FL/fl_draw.H>
#include <cmath>
#include <algorithm>

namespace disgrace_ns {

VUMeter::VUMeter(int x, int y, int w, int h, const char* label, bool horizontal)
    : Fl_Box(x, y, w, h, label), m_horizontal(horizontal) {
    box(FL_FLAT_BOX);
    color(FL_BLACK);
}

void VUMeter::level(float l) {
    if (l < 0.0f) l = 0.0f;
    if (l > 2.0f) l = 2.0f; // Allow some overshoot for display
    
    if (l > m_peak_hold) {
        m_peak_hold = l;
        m_peak_timer = 30; // 30 updates peak hold
    } else if (m_peak_timer > 0) {
        m_peak_timer--;
    } else {
        m_peak_hold *= 0.95f;
    }

    if (std::abs(m_level - l) > 0.001f) {
        m_level = l;
        redraw();
    }
}

void VUMeter::draw() {
    Fl_Box::draw();
    
    int bx = x() + 2;
    int by = y() + 2;
    int bw = w() - 4;
    int bh = h() - 4;

    // Background
    fl_color(FL_BLACK);
    fl_rectf(bx, by, bw, bh);

    float draw_level = std::min(m_level, 2.0f) / 1.5f;
    float peak_norm = std::min(m_peak_hold, 2.0f) / 1.5f;

    if (!m_horizontal) {
        // Vertical drawing (original)
        int bar_h = (int)(bh * draw_level);
        if (bar_h > bh) bar_h = bh;

        for (int i = 0; i < bar_h; ++i) {
            float h_norm = (float)i / bh;
            if (h_norm < 0.5f) fl_color(FL_GREEN);
            else if (h_norm < 0.8f) fl_color(FL_YELLOW);
            else fl_color(FL_RED);
            fl_line(bx, by + bh - i, bx + bw - 1, by + bh - i);
        }

        int peak_y = (int)(bh * peak_norm);
        if (peak_y >= 0 && peak_y < bh) {
            fl_color(FL_WHITE);
            fl_line(bx, by + bh - peak_y, bx + bw - 1, by + bh - peak_y);
        }
        
        fl_color(FL_GRAY0);
        int y_0db = (int)(bh * (1.0f / 1.5f));
        fl_line(bx - 2, by + bh - y_0db, bx + bw + 1, by + bh - y_0db);
    } else {
        // Horizontal drawing (left to right)
        int bar_w = (int)(bw * draw_level);
        if (bar_w > bw) bar_w = bw;

        for (int i = 0; i < bar_w; ++i) {
            float w_norm = (float)i / bw;
            if (w_norm < 0.5f) fl_color(FL_GREEN);
            else if (w_norm < 0.8f) fl_color(FL_YELLOW);
            else fl_color(FL_RED);
            fl_line(bx + i, by, bx + i, by + bh - 1);
        }

        int peak_x = (int)(bw * peak_norm);
        if (peak_x >= 0 && peak_x < bw) {
            fl_color(FL_WHITE);
            fl_line(bx + peak_x, by, bx + peak_x, by + bh - 1);
        }

        fl_color(FL_GRAY0);
        int x_0db = (int)(bw * (1.0f / 1.5f));
        fl_line(bx + x_0db, by - 2, bx + x_0db, by + bh + 1);
    }
}

} // namespace disgrace_ns
