#pragma once
#include <atomic>
#include <cstddef>
#include <string>
#include <memory>
#include "../dsp/dsp_chain.h"

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

    // Master DSP Chain
    void set_effect(size_t index, ::std::unique_ptr<disgrace_ns::DSP> dsp);
    void enable_effect(size_t index, bool en);
    void move_effect_up(size_t index);
    void move_effect_down(size_t index);
    void remove_effect(size_t index);
    disgrace_ns::DSP* get_effect(size_t index) const;

    void load_effect_chain(const std::string& path);
    void save_effect_chain(const std::string& path);

private:
    float soft_clip(float x);

    ::std::atomic<float> m_gain{1.f};
    ::std::atomic<float> m_meter_l{0.f};
    ::std::atomic<float> m_meter_r{0.f};
    disgrace_ns::DSPChain m_chain;
};

} // namespace disgrace_ns
