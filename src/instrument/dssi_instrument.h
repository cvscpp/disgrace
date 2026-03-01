#pragma once
#include "instrument.h"
#include <dssi.h>
#include <dlfcn.h>

namespace disgrace_ns {

class DSSIInstrument : public Instrument {
public:
    DSSIInstrument(double sample_rate);
    ~DSSIInstrument();

    void note_on(uint8_t note, uint8_t velocity) override;
    void note_off() override;
    void set_volume(float vol) override;
    void set_pitch(float freq) override;
    void process(float* l, float* r, size_t nframes) override;

    bool load_plugin(const std::string& path);

protected:
    std::unique_ptr<Voice> create_voice() override { return nullptr; }

private:
    void* m_lib_handle = nullptr;
    const DSSI_Descriptor* m_descriptor = nullptr;
    LADSPA_Handle m_instance = nullptr;
    double m_sample_rate;
};

} // namespace disgrace_ns
