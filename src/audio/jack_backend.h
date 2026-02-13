#pragma once

#include "audio_backend.h"
#include <jack/jack.h>

namespace dg
{

class Engine;

class JackBackend : public AudioBackend
{
public:
    explicit JackBackend(Engine *engine);
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
    jack_port_t *m_out_l;
    jack_port_t *m_out_r;
    jack_port_t* m_in_l;
    jack_port_t* m_in_r;mk
    jack_port_t* m_midi_in;
};

}
