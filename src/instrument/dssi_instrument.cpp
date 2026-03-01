#include "dssi_instrument.h"
#include <iostream>

namespace disgrace_ns {

DSSIInstrument::DSSIInstrument(double sample_rate) : m_sample_rate(sample_rate) {}

DSSIInstrument::~DSSIInstrument() {
    if (m_instance && m_descriptor) {
        m_descriptor->LADSPA_Plugin->cleanup(m_instance);
    }
    if (m_lib_handle) {
        dlclose(m_lib_handle);
    }
}

bool DSSIInstrument::load_plugin(const std::string& path) {
    m_lib_handle = dlopen(path.c_str(), RTLD_NOW);
    if (!m_lib_handle) {
        std::cerr << "Failed to load DSSI plugin: " << dlerror() << std::endl;
        return false;
    }

    DSSI_Descriptor_Function df = (DSSI_Descriptor_Function)dlsym(m_lib_handle, "dssi_descriptor");
    if (!df) {
        std::cerr << "Not a valid DSSI plugin (missing dssi_descriptor)" << std::endl;
        return false;
    }

    m_descriptor = df(0); 
    if (!m_descriptor) return false;

    m_instance = m_descriptor->LADSPA_Plugin->instantiate(m_descriptor->LADSPA_Plugin, (unsigned long)m_sample_rate);
    if (!m_instance) return false;

    if (m_descriptor->LADSPA_Plugin->activate) {
        m_descriptor->LADSPA_Plugin->activate(m_instance);
    }

    return true;
}

void DSSIInstrument::note_on(uint8_t note, uint8_t velocity) {}
void DSSIInstrument::note_off() {}
void DSSIInstrument::set_volume(float vol) {}
void DSSIInstrument::set_pitch(float freq) {}

void DSSIInstrument::process(float* l, float* r, size_t nframes) {
    if (!m_instance || !m_descriptor) {
        for(size_t i=0; i<nframes; ++i) { l[i]=0; r[i]=0; }
        return;
    }
    m_descriptor->LADSPA_Plugin->run(m_instance, (unsigned long)nframes);
}

} // namespace disgrace_ns
