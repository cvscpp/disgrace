#pragma once

#include "audio_backend.h"
#include <jack/jack.h>
#include <vector>

namespace disgrace_ns
{

class Engine;

class JackBackend : public AudioBackend
{
public:
    explicit JackBackend(Engine *engine, 
                         uint32_t num_ins = 2, uint32_t num_outs = 2,
                         uint32_t num_midi_ins = 1, uint32_t num_midi_outs = 1);
    ~JackBackend();

    bool start() override;
    void stop() override;

    uint32_t sample_rate() const override;
    uint32_t buffer_size() const override;

private:
    static int process_callback(jack_nframes_t nframes, void *arg);

    int process(jack_nframes_t nframes);

    Engine *m_engine;

    jack_client_t *m_client;
    
    uint32_t m_num_ins;
    uint32_t m_num_outs;
    uint32_t m_num_midi_ins;
    uint32_t m_num_midi_outs;

    std::vector<jack_port_t*> m_input_ports;
    std::vector<jack_port_t*> m_output_ports;
    std::vector<jack_port_t*> m_midi_input_ports;
    std::vector<jack_port_t*> m_midi_output_ports;
};

} // namespace disgrace_ns
