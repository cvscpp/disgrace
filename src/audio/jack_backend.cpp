#include "jack_backend.h"
#include "../core/engine.h"
#include <jack/midiport.h>
#include <cstring>
#include <cstdio>

namespace disgrace_ns
{

JackBackend::JackBackend(Engine *engine, 
                         uint32_t num_ins, uint32_t num_outs,
                         uint32_t num_midi_ins, uint32_t num_midi_outs)
    : m_engine(engine),
      m_client(nullptr),
      m_num_ins(num_ins),
      m_num_outs(num_outs),
      m_num_midi_ins(num_midi_ins),
      m_num_midi_outs(num_midi_outs)
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

    jack_set_process_callback(m_client, JackBackend::process_callback, this);

    for (uint32_t i = 0; i < m_num_ins; ++i) {
        char name[32];
        snprintf(name, sizeof(name), "in_%u", i + 1);
        m_input_ports.push_back(jack_port_register(m_client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0));
    }

    for (uint32_t i = 0; i < m_num_outs; ++i) {
        char name[32];
        snprintf(name, sizeof(name), "out_%u", i + 1);
        m_output_ports.push_back(jack_port_register(m_client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0));
    }

    for (uint32_t i = 0; i < m_num_midi_ins; ++i) {
        char name[32];
        snprintf(name, sizeof(name), "midi_in_%u", i + 1);
        m_midi_input_ports.push_back(jack_port_register(m_client, name, JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0));
    }

    for (uint32_t i = 0; i < m_num_midi_outs; ++i) {
        char name[32];
        snprintf(name, sizeof(name), "midi_out_%u", i + 1);
        m_midi_output_ports.push_back(jack_port_register(m_client, name, JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0));
    }

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
        m_input_ports.clear();
        m_output_ports.clear();
        m_midi_input_ports.clear();
        m_midi_output_ports.clear();
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

bool JackBackend::is_active() const
{
    return m_client != nullptr;
}

int JackBackend::process_callback(jack_nframes_t nframes, void *arg)
{
    return static_cast<JackBackend *>(arg)->process(nframes);
}

int JackBackend::process(jack_nframes_t nframes)
{
    std::vector<float*> out_bufs;
    for (auto* port : m_output_ports) {
        float* buf = (float*)jack_port_get_buffer(port, nframes);
        if (buf) std::memset(buf, 0, sizeof(float) * nframes);
        out_bufs.push_back(buf);
    }

    std::vector<float*> in_bufs;
    for (auto* port : m_input_ports) {
        in_bufs.push_back((float*)jack_port_get_buffer(port, nframes));
    }

    // Process MIDI inputs
    for (auto* port : m_midi_input_ports) {
        void* midi_buf = jack_port_get_buffer(port, nframes);
        if (!midi_buf) continue;

        uint32_t event_count = jack_midi_get_event_count(midi_buf);
        for (uint32_t i = 0; i < event_count; ++i)
        {
            jack_midi_event_t ev;
            jack_midi_event_get(&ev, midi_buf, i);
            m_engine->handle_midi(ev.buffer, ev.size);
        }
    }

    // Process MIDI outputs (clear for now)
    for (auto* port : m_midi_output_ports) {
        void* midi_buf = jack_port_get_buffer(port, nframes);
        if (midi_buf) jack_midi_clear_buffer(midi_buf);
    }

    return 0;
}

} // namespace disgrace_ns
