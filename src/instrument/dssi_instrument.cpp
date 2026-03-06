#include "dssi_instrument.h"
#include <iostream>
#include <cmath>

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

bool DSSIInstrument::load_plugin(const std::string& path, int index) {
    if (m_lib_handle) {
        if (m_instance) m_descriptor->LADSPA_Plugin->cleanup(m_instance);
        dlclose(m_lib_handle);
        m_lib_handle = nullptr;
        m_instance = nullptr;
        m_control_indices.clear();
        m_port_values.clear();
    }

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

    m_descriptor = df(index); 
    if (!m_descriptor) return false;

    const LADSPA_Descriptor* ladspa = m_descriptor->LADSPA_Plugin;
    m_instance = ladspa->instantiate(ladspa, (unsigned long)m_sample_rate);
    if (!m_instance) return false;

    set_plugin_name(ladspa->Name);

    // Discover control ports
    m_port_values.resize(ladspa->PortCount, 0.0f);
    for (unsigned long i = 0; i < ladspa->PortCount; ++i) {
        LADSPA_PortDescriptor d = ladspa->PortDescriptors[i];
        if (LADSPA_IS_PORT_CONTROL(d) && LADSPA_IS_PORT_INPUT(d)) {
            m_control_indices.push_back((int)i);
            
            // Set default value if hint provided
            LADSPA_PortRangeHint hint = ladspa->PortRangeHints[i];
            float val = 0.0f;
            if (LADSPA_IS_HINT_HAS_DEFAULT(hint.HintDescriptor)) {
                if (LADSPA_IS_HINT_DEFAULT_0(hint.HintDescriptor)) val = 0.0f;
                else if (LADSPA_IS_HINT_DEFAULT_1(hint.HintDescriptor)) val = 1.0f;
                else if (LADSPA_IS_HINT_DEFAULT_100(hint.HintDescriptor)) val = 100.0f;
                else if (LADSPA_IS_HINT_DEFAULT_440(hint.HintDescriptor)) val = 440.0f;
                else if (LADSPA_IS_HINT_DEFAULT_MINIMUM(hint.HintDescriptor)) val = hint.LowerBound;
                else if (LADSPA_IS_HINT_DEFAULT_MAXIMUM(hint.HintDescriptor)) val = hint.UpperBound;
                else if (LADSPA_IS_HINT_DEFAULT_LOW(hint.HintDescriptor)) val = hint.LowerBound * 0.75f + hint.UpperBound * 0.25f;
                else if (LADSPA_IS_HINT_DEFAULT_MIDDLE(hint.HintDescriptor)) val = hint.LowerBound * 0.5f + hint.UpperBound * 0.5f;
                else if (LADSPA_IS_HINT_DEFAULT_HIGH(hint.HintDescriptor)) val = hint.LowerBound * 0.25f + hint.UpperBound * 0.75f;
            }
            m_port_values[i] = val;
            ladspa->connect_port(m_instance, i, &m_port_values[i]);
        }
    }

    if (ladspa->activate) {
        ladspa->activate(m_instance);
    }

    return true;
}

Instrument::Parameter DSSIInstrument::get_parameter(size_t index) const {
    if (index >= m_control_indices.size()) return {};
    int port_idx = m_control_indices[index];
    const LADSPA_Descriptor* ladspa = m_descriptor->LADSPA_Plugin;
    
    Instrument::Parameter p;
    p.index = (int)index;
    p.name = ladspa->PortNames[port_idx];
    p.min = ladspa->PortRangeHints[port_idx].LowerBound;
    p.max = ladspa->PortRangeHints[port_idx].UpperBound;
    p.value = m_port_values[port_idx];
    
    // Sanity check for bounds if not provided
    if (!LADSPA_IS_HINT_BOUNDED_BELOW(ladspa->PortRangeHints[port_idx].HintDescriptor)) p.min = 0.0f;
    if (!LADSPA_IS_HINT_BOUNDED_ABOVE(ladspa->PortRangeHints[port_idx].HintDescriptor)) p.max = 1.0f;
    if (p.max <= p.min) p.max = p.min + 1.0f;

    return p;
}

void DSSIInstrument::set_parameter(size_t index, float value) {
    if (index >= m_control_indices.size()) return;
    int port_idx = m_control_indices[index];
    m_port_values[port_idx] = value;
}

    void DSSIInstrument::note_on(uint8_t note, uint8_t velocity, size_t column_index, size_t)
 {
    // DSSI specific note on handling would go here (MIDI or OSC)
}

void DSSIInstrument::note_off(size_t column_index) {}
void DSSIInstrument::panic() {}
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
