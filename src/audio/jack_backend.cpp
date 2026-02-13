#include "jack_backend.h"
#include "../core/engine.h"

#include <cstring>

namespace dg
{

JackBackend::JackBackend(Engine *engine)
    : m_engine(engine),
      m_client(nullptr),
      m_out_l(nullptr),
      m_out_r(nullptr)
{
}

JackBackend::~JackBackend()
{
    stop();
}

bool JackBackend::start()
{
    m_client = jack_client_open("disgrace", JackNullOption, nullptr);
    if (!m_client)
        return false;

    jack_set_process_callback(m_client, process_callback, this);

    m_out_l = jack_port_register(m_client,
                                 "out_l",
                                 JACK_DEFAULT_AUDIO_TYPE,
                                 JackPortIsOutput,
                                 0);

    m_out_r = jack_port_register(m_client,
                                 "out_r",
                                 JACK_DEFAULT_AUDIO_TYPE,
                                 JackPortIsOutput,
                                 0);

    m_midi_in = jack_port_register(
        m_client,
        "midi_in",
        JACK_DEFAULT_MIDI_TYPE,
        JackPortIsInput,
        0);

    if (jack_activate(m_client))
        return false;

    return true;
}

void JackBackend::stop()
{
    if (m_client)
    {
        jack_client_close(m_client);
        m_client = nullptr;
    }
}

uint32_t JackBackend::sample_rate() const
{
    return m_client ? jack_get_sample_rate(m_client) : 0;
}

uint32_t JackBackend::buffer_size() const
{
    return m_client ? jack_get_buffer_size(m_client) : 0;
}

int JackBackend::process_callback(jack_nframes_t nframes, void *arg)
{
    return static_cast<JackBackend *>(arg)->process(nframes);
}

int JackBackend::process(jack_nframes_t nframes)
{
    float *out_l = (float *)jack_port_get_buffer(m_out_l, nframes);
    float *out_r = (float *)jack_port_get_buffer(m_out_r, nframes);

    std::memset(out_l, 0, sizeof(float) * nframes);
    std::memset(out_r, 0, sizeof(float) * nframes);

    float* in_l = (float*)
    jack_port_get_buffer(m_in_l, nframes);
    float* in_r = (float*)
    jack_port_get_buffer(m_in_r, nframes);

    m_engine->record_input(in_l, in_r, nframes);
    void* midi_buf =
    jack_port_get_buffer(m_midi_in, nframes);

    uint32_t event_count =
    jack_midi_get_event_count(midi_buf);

    for (uint32_t i = 0; i < event_count; ++i)
    {
        jack_midi_event_t ev;
        jack_midi_event_get(&ev, midi_buf, i);

        m_engine->handle_midi(ev.buffer,
                              ev.size);
    }

    // Later:
    // m_engine->process_audio(out_l, out_r, nframes);

    return 0;
}

}
