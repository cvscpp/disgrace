#pragma once
#include "dsp.h"
#include <nlohmann/json.hpp>

namespace disgrace_ns
{

class GainDSP : public disgrace_ns::DSP
{
public:
    float gain = 1.0f;

    std::string name() const override { return "Gain"; }
    std::string type_name() const override { return "Gain"; }

    void process(float* l,
                 float* r,
                 size_t nframes) override
    {
        if (m_bypassed) return;
        for (size_t i = 0; i < nframes; ++i)
        {
            l[i] *= gain;
            r[i] *= gain;
        }
    }

    std::string get_state() override {
        nlohmann::json j;
        j["gain"] = gain;
        j["bypassed"] = m_bypassed;
        return j.dump();
    }

    void set_state(const std::string& state) override {
        auto j = nlohmann::json::parse(state);
        if (j.contains("gain")) gain = j["gain"];
        if (j.contains("bypassed")) m_bypassed = j["bypassed"];
    }
};

} // namespace disgrace_ns
