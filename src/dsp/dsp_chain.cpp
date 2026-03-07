#include "dsp_chain.h"
#include "gain.h"
#include "delay.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace disgrace_ns
{

static std::unique_ptr<DSP> create_dsp(const std::string& type) {
    if (type == "Gain") return std::make_unique<GainDSP>();
    if (type == "Delay") return std::make_unique<DelayDSP>();
    return nullptr;
}

void disgrace_ns::DSPChain::process(float* l,
                       float* r,
                       size_t nframes)
{
    for (size_t i = 0; i < MAX_INSERTS; ++i)
    {
        if (m_enabled[i] && m_effects[i])
            m_effects[i]->process(l, r, nframes);
    }
}

void disgrace_ns::DSPChain::set(size_t index, ::std::unique_ptr<disgrace_ns::DSP> dsp)
{
    if (index < MAX_INSERTS)
        m_effects[index] = std::move(dsp);
}

void disgrace_ns::DSPChain::enable(size_t index, bool en)
{
    if (index < MAX_INSERTS)
        m_enabled[index] = en;
}

void disgrace_ns::DSPChain::move_up(size_t index)
{
    if (index > 0 && index < MAX_INSERTS) {
        std::swap(m_effects[index], m_effects[index - 1]);
        std::swap(m_enabled[index], m_enabled[index - 1]);
    }
}

void disgrace_ns::DSPChain::move_down(size_t index)
{
    if (index < MAX_INSERTS - 1) {
        std::swap(m_effects[index], m_effects[index + 1]);
        std::swap(m_enabled[index], m_enabled[index + 1]);
    }
}

void disgrace_ns::DSPChain::remove(size_t index)
{
    if (index < MAX_INSERTS) {
        m_effects[index].reset();
        m_enabled[index] = false;
    }
}

void disgrace_ns::DSPChain::save_chain(const std::string& path)
{
    nlohmann::json j = nlohmann::json::array();
    for (size_t i = 0; i < MAX_INSERTS; ++i) {
        if (m_effects[i]) {
            nlohmann::json fx;
            fx["type"] = m_effects[i]->type_name();
            fx["state"] = nlohmann::json::parse(m_effects[i]->get_state());
            fx["enabled"] = m_enabled[i];
            j.push_back(fx);
        }
    }
    std::ofstream f(path);
    if (f.is_open()) f << j.dump(4);
}

void disgrace_ns::DSPChain::load_chain(const std::string& path)
{
    std::ifstream f(path);
    if (!f.is_open()) return;
    
    try {
        auto j = nlohmann::json::parse(f);
        if (j.is_array()) {
            // Clear current chain
            for (size_t i = 0; i < MAX_INSERTS; ++i) {
                m_effects[i].reset();
                m_enabled[i] = false;
            }

            size_t idx = 0;
            for (const auto& fx : j) {
                if (idx >= MAX_INSERTS) break;
                if (fx.contains("type")) {
                    auto dsp = create_dsp(fx["type"]);
                    if (dsp) {
                        if (fx.contains("state")) dsp->set_state(fx["state"].dump());
                        m_effects[idx] = std::move(dsp);
                        m_enabled[idx] = fx.value("enabled", true);
                        idx++;
                    }
                }
            }
        }
    } catch (...) {}
}

} // namespace disgrace_ns
