#pragma once
#include "instrument.h"
#include <dssi.h>
#include <dlfcn.h>
#include <vector>

namespace disgrace_ns {

class DSSIInstrument : public Instrument {
public:
    DSSIInstrument(double sample_rate);
    ~DSSIInstrument();

    void note_on(uint8_t note, uint8_t velocity, size_t offset_samples = 0) override;
    void note_off() override;
    void set_volume(float vol) override;
    void set_pitch(float freq) override;
    void process(float* l, float* r, size_t nframes) override;

    bool load_plugin(const std::string& path, int index = 0);

    size_t parameter_count() const override { return m_control_indices.size(); }
    Parameter get_parameter(size_t index) const override;
    void set_parameter(size_t index, float value) override;

protected:
    std::unique_ptr<Voice> create_voice() override { return nullptr; }

private:
    void* m_lib_handle = nullptr;
    const DSSI_Descriptor* m_descriptor = nullptr;
    LADSPA_Handle m_instance = nullptr;
    double m_sample_rate;

    std::vector<float> m_port_values;
    std::vector<int>   m_control_indices;
};

} // namespace disgrace_ns
