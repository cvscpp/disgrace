#pragma once
#include "dsp.h"
#include <array>
#include <nlohmann/json.hpp>

namespace disgrace_ns
{

class DelayDSP : public disgrace_ns::DSP
{
public:
    static constexpr size_t MAX_DELAY = 48000;

    float feedback = 0.4f;
    float mix = 0.3f;

    std::string name() const override { return "Delay"; }
    std::string type_name() const override { return "Delay"; }

    void process(float* l,
                 float* r,
                 size_t nframes) override
    {
        if (m_bypassed) return;
        for (size_t i = 0; i < nframes; ++i)
        {
            float dl = m_buffer_l[m_pos];
            float dr = m_buffer_r[m_pos];

            m_buffer_l[m_pos] = l[i] + dl * feedback;
            m_buffer_r[m_pos] = r[i] + dr * feedback;

            l[i] = l[i] * (1 - mix) + dl * mix;
            r[i] = r[i] * (1 - mix) + dr * mix;

            m_pos = (m_pos + 1) % MAX_DELAY;
        }
    }

    std::string get_state() override {
        nlohmann::json j;
        j["feedback"] = feedback;
        j["mix"] = mix;
        j["bypassed"] = m_bypassed;
        return j.dump();
    }

    void set_state(const std::string& state) override {
        auto j = nlohmann::json::parse(state);
        if (j.contains("feedback")) feedback = j["feedback"];
        if (j.contains("mix")) mix = j["mix"];
        if (j.contains("bypassed")) m_bypassed = j["bypassed"];
    }

private:
    ::std::array<float, MAX_DELAY> m_buffer_l{};
    ::std::array<float, MAX_DELAY> m_buffer_r{};
    size_t m_pos = 0;
};

} // namespace disgrace_ns
