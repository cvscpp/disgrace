#pragma once
#include <atomic>
#include <cstddef>

namespace disgrace_ns
{

class MasterBus
{
public:
    void set_gain(float g);
    float gain() const;

    void process(float* l,
                 float* r,
                 size_t nframes);

    float meter_l() const;
    float meter_r() const;

private:
    float soft_clip(float x);

    ::std::atomic<float> m_gain{1.f};
    ::std::atomic<float> m_meter_l{0.f};
    ::std::atomic<float> m_meter_r{0.f};
};

} // namespace disgrace_ns
