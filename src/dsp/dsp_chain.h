#pragma once

#include "dsp.h"
#include <array>
#include <memory>

namespace disgrace_ns
{

constexpr size_t MAX_INSERTS = 32;

class DSPChain
{
public:
    void process(float* l,
                 float* r,
                 size_t nframes);

    void set(size_t index, ::std::unique_ptr<DSP> dsp);
    void enable(size_t index, bool en);
    void move_up(size_t index);
    void move_down(size_t index);
    void remove(size_t index);

    const ::std::array<::std::unique_ptr<disgrace_ns::DSP>, MAX_INSERTS>& effects() const { return m_effects; }
    bool is_enabled(size_t index) const { return index < MAX_INSERTS && m_enabled[index]; }

    void save_chain(const std::string& path);
    void load_chain(const std::string& path);

    void to_json(void* j_ptr) const; // j_ptr is nlohmann::json*
    void from_json(const void* j_ptr); // j_ptr is const nlohmann::json*

private:
    ::std::array<::std::unique_ptr<disgrace_ns::DSP>, MAX_INSERTS> m_effects{};
    ::std::array<bool, MAX_INSERTS> m_enabled{};
};

} // namespace disgrace_ns
