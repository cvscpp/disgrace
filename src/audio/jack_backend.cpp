/*
 * Disgrace - Digital Audio Workstation
 * Copyright (C) 2025  Miroslav Shaltev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "jack_backend.h"
#include "../core/engine.h"
#include <jack/midiport.h>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>

namespace disgrace_ns
{

namespace {

// JACK port names are "<client>:<port>".  The client tells us which device
// (card) a physical port belongs to.
std::string jack_client_of(const char* port)
{
    if (!port) return {};
    const char* colon = std::strchr(port, ':');
    return colon ? std::string(port, (size_t)(colon - port)) : std::string(port);
}

// Pick the one physical device to wire our ports to.  jackd's driver client
// is called "system"; when the graph carries several physical devices
// (multi-card setups, PipeWire, composite servers) jack_get_ports() returns
// all of them in an unspecified order, and blindly connecting the first N
// entries can spread a stereo pair across two cards — the mix then comes out
// of a card jackd is not driving.  Restrict the auto-connect to a single
// client so at least the whole output pair always stays on one card.
std::string choose_physical_device(const char** ports)
{
    if (!ports || !ports[0]) return {};
    std::string chosen = jack_client_of(ports[0]);
    for (size_t i = 0; ports[i]; ++i) {
        if (jack_client_of(ports[i]) == "system") {
            chosen = "system";
            break;
        }
    }
    return chosen;
}

} // namespace

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

    // JACK MIDI ports are expected to be available whenever the JACK backend is active.
    // Some configuration paths can leave these counts at 0, so promote them to 1 here.
    m_num_midi_ins = std::max(1u, m_num_midi_ins);
    m_num_midi_outs = std::max(1u, m_num_midi_outs);

    for (uint32_t i = 0; i < m_num_ins; ++i) {
        char name[32];
        snprintf(name, sizeof(name), "in_%u", i + 1);
        auto* port = jack_port_register(m_client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        if (!port) {
            std::cerr << "[JACK] Failed to register audio input port " << name << std::endl;
            stop();
            return false;
        }
        m_input_ports.push_back(port);
    }

    for (uint32_t i = 0; i < m_num_outs; ++i) {
        char name[32];
        snprintf(name, sizeof(name), "out_%u", i + 1);
        auto* port = jack_port_register(m_client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        if (!port) {
            std::cerr << "[JACK] Failed to register audio output port " << name << std::endl;
            stop();
            return false;
        }
        m_output_ports.push_back(port);
    }

    for (uint32_t i = 0; i < m_num_midi_ins; ++i) {
        char name[32];
        snprintf(name, sizeof(name), "midi_in_%u", i + 1);
        auto* port = jack_port_register(m_client, name, JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
        if (!port) {
            std::cerr << "[JACK] Failed to register MIDI input port " << name << std::endl;
            stop();
            return false;
        }
        m_midi_input_ports.push_back(port);
    }

    for (uint32_t i = 0; i < m_num_midi_outs; ++i) {
        char name[32];
        snprintf(name, sizeof(name), "midi_out_%u", i + 1);
        auto* port = jack_port_register(m_client, name, JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
        if (!port) {
            std::cerr << "[JACK] Failed to register MIDI output port " << name << std::endl;
            stop();
            return false;
        }
        m_midi_output_ports.push_back(port);
    }

    if (jack_activate(m_client))
        return false;

    // Pre-allocate RT buffer pointer arrays after port count is known.
    m_out_bufs_rt.resize(m_num_outs, nullptr);
    m_in_bufs_rt.resize(m_num_ins, nullptr);

    // Auto-connect output ports to physical playback ports.  Keep every
    // connection on one physical device so a mix (including DSSI plugin
    // output) cannot leak onto another sound card when the graph has more
    // than one physical device.
    const char **ports = jack_get_ports(m_client, nullptr, nullptr, JackPortIsPhysical | JackPortIsInput);
    if (ports) {
        const std::string device = choose_physical_device(ports);
        uint32_t connected = 0;
        for (size_t i = 0; ports[i] && connected < m_num_outs; ++i) {
            if (jack_client_of(ports[i]) != device) continue;
            const int rc = jack_connect(m_client, jack_port_name(m_output_ports[connected]), ports[i]);
            if (rc != 0 && rc != EEXIST) {
                std::cerr << "[JACK] could not connect "
                          << jack_port_name(m_output_ports[connected]) << " to "
                          << ports[i] << " (error " << rc << ")" << std::endl;
            } else {
                std::cerr << "[JACK] " << jack_port_name(m_output_ports[connected])
                          << " -> " << ports[i] << std::endl;
            }
            ++connected;
        }
        if (connected < m_num_outs) {
            std::cerr << "[JACK] auto-connected " << connected << " of "
                      << m_num_outs << " output ports on device '"
                      << device << "'" << std::endl;
        }
        jack_free(ports);
    }

    // Auto-connect physical capture ports to input ports (same device).
    ports = jack_get_ports(m_client, nullptr, nullptr, JackPortIsPhysical | JackPortIsOutput);
    if (ports) {
        const std::string device = choose_physical_device(ports);
        uint32_t connected = 0;
        for (size_t i = 0; ports[i] && connected < m_num_ins; ++i) {
            if (jack_client_of(ports[i]) != device) continue;
            const int rc = jack_connect(m_client, ports[i], jack_port_name(m_input_ports[connected]));
            if (rc != 0 && rc != EEXIST) {
                std::cerr << "[JACK] could not connect " << ports[i] << " to "
                          << jack_port_name(m_input_ports[connected])
                          << " (error " << rc << ")" << std::endl;
            }
            ++connected;
        }
        jack_free(ports);
    }

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
        m_out_bufs_rt.clear();
        m_in_bufs_rt.clear();
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
    // Fill pre-allocated buffer pointer arrays — no heap allocation in the RT thread.
    for (uint32_t i = 0; i < (uint32_t)m_output_ports.size(); ++i) {
        float* buf = (float*)jack_port_get_buffer(m_output_ports[i], nframes);
        if (buf) std::memset(buf, 0, sizeof(float) * nframes);
        m_out_bufs_rt[i] = buf;
    }

    for (uint32_t i = 0; i < (uint32_t)m_input_ports.size(); ++i) {
        m_in_bufs_rt[i] = (float*)jack_port_get_buffer(m_input_ports[i], nframes);
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

    // Process MIDI outputs
    MidiMessage midi_messages[1024];
    size_t midi_message_count = 0;
    while (midi_message_count < (sizeof(midi_messages) / sizeof(midi_messages[0])) &&
           m_engine->m_midi_out_queue.pop(midi_messages[midi_message_count])) {
        ++midi_message_count;
    }

    for (auto* port : m_midi_output_ports) {
        void* midi_buf = jack_port_get_buffer(port, nframes);
        if (!midi_buf) continue;
        jack_midi_clear_buffer(midi_buf);

        for (size_t i = 0; i < midi_message_count; ++i) {
            const MidiMessage& out_msg = midi_messages[i];
            // Skip system messages (0xF0+) which have variable/undefined length here.
            if (out_msg.status >= 0xF0) continue;
            // Program Change (0xC0) and Channel Aftertouch (0xD0) are 2-byte
            // messages; everything else we emit is 3 bytes. Reserving 3 bytes for
            // a 2-byte message would inject a spurious trailing 0x00.
            size_t len = ((out_msg.status & 0xF0) == 0xC0 ||
                          (out_msg.status & 0xF0) == 0xD0) ? 2 : 3;
            unsigned char* buf = jack_midi_event_reserve(midi_buf, 0, len);
            if (buf) {
                buf[0] = out_msg.status;
                buf[1] = out_msg.data1;
                if (len == 3) buf[2] = out_msg.data2;
            }
        }
    }

    if (m_out_bufs_rt.size() >= 2) {
        m_engine->process_audio(m_in_bufs_rt.data(), (uint32_t)m_in_bufs_rt.size(),
                                m_out_bufs_rt.data(), (uint32_t)m_out_bufs_rt.size(), nframes);
    }

    return 0;
}

} // namespace disgrace_ns
