#pragma once

#include <FL/Fl_Box.H>
#include <vector>
#include <memory>
#include "../analysis/fft_analyzer.h"

namespace disgrace_ns {

class Engine;

class SpectralView : public Fl_Box {
public:
    SpectralView(int x, int y, int w, int h, Engine& engine);
    ~SpectralView();

    void update();
    void draw() override;

private:
    Engine& m_engine;
    std::unique_ptr<FFTAnalyzer> m_analyzer;
    std::vector<float> m_history;
    std::vector<float> m_fft_input;
    size_t m_fft_size = 1024;
};

} // namespace disgrace_ns
