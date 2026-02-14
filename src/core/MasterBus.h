#pragma once
#include <atomic>
#include <cstddef>

class MasterBus
{
public:
    void set_gain(float g);
    float gain() const;

    void process(float* l,
                 float* r,
                 size_t nframes);

    float meter() const;

private:
    float soft_clip(float x);

    std::atomic<float> m_gain{1.f};
    std::atomic<float> m_meter{0.f};
};
