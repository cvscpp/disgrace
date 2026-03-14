#pragma once

#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

namespace disgrace_ns {

class Engine;

class VUMeter : public Fl_Box {
public:
    VUMeter(int x, int y, int w, int h, Engine& engine, const char* label = nullptr, bool horizontal = false);

    void level(float l);
    float level() const { return m_level; }

    void draw() override;

private:
    Engine& m_engine;
    float m_level = 0.0f;
    float m_peak_hold = 0.0f;
    int m_peak_timer = 0;
    bool m_horizontal = false;
};

} // namespace disgrace_ns
