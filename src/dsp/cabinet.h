#pragma once
#include "dsp.h"
#include <nlohmann/json.hpp>
#include <cmath>
#include <vector>
#include <string>

namespace disgrace_ns
{

enum class CabinetType {
    Cab_1x12,
    Cab_2x12,
    Cab_4x12,
    Cab_8x12,
    Cab_4x10,
    Cab_1x15,
    Cab_2x15,
    Cab_1x15_4x10
};

class CabinetDSP : public disgrace_ns::DSP
{
public:
    struct Biquad {
        float b0, b1, b2, a1, a2;
        float x1_l, x2_l, y1_l, y2_l;
        float x1_r, x2_r, y1_r, y2_r;

        void reset() {
            x1_l = x2_l = y1_l = y2_l = 0;
            x1_r = x2_r = y1_r = y2_r = 0;
        }

        void set_lpf(float freq, float q, float sr) {
            float w0 = 2.0f * M_PI * freq / sr;
            float cos_w0 = std::cos(w0);
            float alpha = std::sin(w0) / (2.0f * q);
            float b0_raw = (1.0f - cos_w0) / 2.0f;
            float b1_raw = 1.0f - cos_w0;
            float b2_raw = (1.0f - cos_w0) / 2.0f;
            float a0_raw = 1.0f + alpha;
            float a1_raw = -2.0f * cos_w0;
            float a2_raw = 1.0f - alpha;
            b0 = b0_raw / a0_raw; b1 = b1_raw / a0_raw; b2 = b2_raw / a0_raw;
            a1 = a1_raw / a0_raw; a2 = a2_raw / a0_raw;
        }

        void set_hpf(float freq, float q, float sr) {
            float w0 = 2.0f * M_PI * freq / sr;
            float cos_w0 = std::cos(w0);
            float alpha = std::sin(w0) / (2.0f * q);
            float b0_raw = (1.0f + cos_w0) / 2.0f;
            float b1_raw = -(1.0f + cos_w0);
            float b2_raw = (1.0f + cos_w0) / 2.0f;
            float a0_raw = 1.0f + alpha;
            float a1_raw = -2.0f * cos_w0;
            float a2_raw = 1.0f - alpha;
            b0 = b0_raw / a0_raw; b1 = b1_raw / a0_raw; b2 = b2_raw / a0_raw;
            a1 = a1_raw / a0_raw; a2 = a2_raw / a0_raw;
        }

        void set_peaking(float freq, float q, float gain_db, float sr) {
            float a = std::pow(10.0f, gain_db / 40.0f);
            float w0 = 2.0f * M_PI * freq / sr;
            float alpha = std::sin(w0) / (2.0f * q);
            float cos_w0 = std::cos(w0);
            float b0_raw = 1.0f + alpha * a;
            float b1_raw = -2.0f * cos_w0;
            float b2_raw = 1.0f - alpha * a;
            float a0_raw = 1.0f + alpha / a;
            float a1_raw = -2.0f * cos_w0;
            float a2_raw = 1.0f - alpha / a;
            b0 = b0_raw / a0_raw; b1 = b1_raw / a0_raw; b2 = b2_raw / a0_raw;
            a1 = a1_raw / a0_raw; a2 = a2_raw / a0_raw;
        }

        inline float process_l(float in) {
            float out = b0 * in + b1 * x1_l + b2 * x2_l - a1 * y1_l - a2 * y2_l;
            x2_l = x1_l; x1_l = in; y2_l = y1_l; y1_l = out;
            return out;
        }

        inline float process_r(float in) {
            float out = b0 * in + b1 * x1_r + b2 * x2_r - a1 * y1_r - a2 * y2_r;
            x2_r = x1_r; x1_r = in; y2_r = y1_r; y1_r = out;
            return out;
        }
    };

    CabinetDSP() {
        m_filters.resize(4);
        update_filters();
    }

    CabinetType type = CabinetType::Cab_4x12;

    std::string name() const override { return "Cabinet"; }
    std::string type_name() const override { return "Cabinet"; }

    static std::vector<std::string> get_type_names() {
        return { "1 x 12\"", "2 x 12\"", "4 x 12\"", "8 x 12\"", "4 x 10\"", "1 x 15\"", "2 x 15\"", "1 x 15\" + 4 x 10\"" };
    }

    void process(float* l, float* r, size_t nframes) override
    {
        if (m_bypassed) return;
        for (size_t i = 0; i < nframes; ++i)
        {
            float out_l = l[i];
            float out_r = r[i];
            for (auto& f : m_filters) {
                out_l = f.process_l(out_l);
                out_r = f.process_r(out_r);
            }
            l[i] = out_l;
            r[i] = out_r;
        }
    }

    std::string get_state() override {
        nlohmann::json j;
        j["type"] = (int)type;
        j["bypassed"] = m_bypassed;
        return j.dump();
    }

    void set_state(const std::string& state) override {
        try {
            auto j = nlohmann::json::parse(state);
            if (j.contains("type")) type = (CabinetType)j["type"];
            if (j.contains("bypassed")) m_bypassed = j["bypassed"];
            update_filters();
        } catch(...) {}
    }

    std::vector<std::string> get_presets() override {
        return get_type_names();
    }

    void load_preset(const std::string& name) override {
        auto names = get_type_names();
        for(size_t i=0; i<names.size(); ++i) {
            if (names[i] == name) {
                type = (CabinetType)i;
                update_filters();
                break;
            }
        }
    }

private:
    void update_filters() {
        float sr = 44100.0f;
        for(auto& f : m_filters) f.reset();

        switch(type) {
            case CabinetType::Cab_1x12:
                m_filters[0].set_hpf(100.0f, 0.707f, sr);
                m_filters[1].set_lpf(5000.0f, 0.707f, sr);
                m_filters[2].set_peaking(2500.0f, 1.0f, 3.0f, sr);
                m_filters[3].set_peaking(400.0f, 1.0f, -2.0f, sr);
                break;
            case CabinetType::Cab_2x12:
                m_filters[0].set_hpf(90.0f, 0.707f, sr);
                m_filters[1].set_lpf(6000.0f, 0.707f, sr);
                m_filters[2].set_peaking(3000.0f, 1.0f, 2.0f, sr);
                m_filters[3].set_peaking(500.0f, 1.0f, 1.0f, sr);
                break;
            case CabinetType::Cab_4x12:
                m_filters[0].set_hpf(80.0f, 0.707f, sr);
                m_filters[1].set_lpf(5000.0f, 0.707f, sr);
                m_filters[2].set_peaking(2000.0f, 0.707f, 4.0f, sr);
                m_filters[3].set_peaking(100.0f, 1.0f, 3.0f, sr);
                break;
            case CabinetType::Cab_8x12:
                m_filters[0].set_hpf(70.0f, 0.707f, sr);
                m_filters[1].set_lpf(4500.0f, 0.707f, sr);
                m_filters[2].set_peaking(1500.0f, 0.5f, 5.0f, sr);
                m_filters[3].set_peaking(150.0f, 0.707f, 4.0f, sr);
                break;
            case CabinetType::Cab_4x10:
                m_filters[0].set_hpf(120.0f, 0.707f, sr);
                m_filters[1].set_lpf(7000.0f, 0.707f, sr);
                m_filters[2].set_peaking(3500.0f, 1.2f, 3.0f, sr);
                m_filters[3].set_peaking(800.0f, 1.0f, -3.0f, sr);
                break;
            case CabinetType::Cab_1x15:
                m_filters[0].set_hpf(50.0f, 0.707f, sr);
                m_filters[1].set_lpf(3500.0f, 0.707f, sr);
                m_filters[2].set_peaking(1000.0f, 0.707f, 2.0f, sr);
                m_filters[3].set_peaking(200.0f, 1.0f, 4.0f, sr);
                break;
            case CabinetType::Cab_2x15:
                m_filters[0].set_hpf(40.0f, 0.707f, sr);
                m_filters[1].set_lpf(3000.0f, 0.707f, sr);
                m_filters[2].set_peaking(800.0f, 0.5f, 3.0f, sr);
                m_filters[3].set_peaking(150.0f, 0.707f, 5.0f, sr);
                break;
            case CabinetType::Cab_1x15_4x10:
                m_filters[0].set_hpf(45.0f, 0.707f, sr);
                m_filters[1].set_lpf(6500.0f, 0.707f, sr);
                m_filters[2].set_peaking(3000.0f, 1.0f, 2.0f, sr);
                m_filters[3].set_peaking(100.0f, 0.707f, 4.0f, sr);
                break;
        }
    }
    std::vector<Biquad> m_filters;
};

} // namespace disgrace_ns
