/*
 * Disgrace - Digital Audio Workstation
 * Copyright (C) 2025  Miroslav Shaltev
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * dssi_sandbox — minimal subprocess that hosts a single DSSI/LADSPA plugin
 * in isolation.  A crash inside the plugin code cannot propagate to the
 * main Disgrace process.
 *
 * Usage:
 *   dssi_sandbox <shm_name> <plugin_path> <plugin_index> <sample_rate>
 *
 * Startup protocol (stdout, line-oriented):
 *   On success:
 *     OK\n
 *     NAME <plugin_name>\n
 *     HAS_SYNTH <0|1>\n
 *     CTRL_COUNT <n>\n
 *     CTRL <seq_idx> <port_idx> <min> <max> <default> <name>\n  (×n)
 *     AUDIO_L <port_idx|-1>\n
 *     AUDIO_R <port_idx|-1>\n
 *     READY\n
 *   On failure:
 *     ERROR <message>\n
 *
 * After writing READY the process enters its RT loop and no longer
 * writes to stdout.  The host reads until the READY line.
 */

#include "dssi_bridge_shm.h"

#include <dssi.h>
#include <dlfcn.h>
#include <alsa/asoundlib.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cerrno>
#include <algorithm>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

using namespace disgrace_ns;

// ─── Helpers ──────────────────────────────────────────────────────────────────

static float apply_sr_hint(float v, const LADSPA_PortRangeHint& h, double sr)
{
    return LADSPA_IS_HINT_SAMPLE_RATE(h.HintDescriptor) ? v * (float)sr : v;
}

static float default_port_value(const LADSPA_PortRangeHint& h, double sr)
{
    float lo = apply_sr_hint(h.LowerBound, h, sr);
    float hi = apply_sr_hint(h.UpperBound, h, sr);
    bool  lg = LADSPA_IS_HINT_LOGARITHMIC(h.HintDescriptor);
    auto  lerp = [&](float t) -> float {
        if (lg && lo > 0.f && hi > 0.f)
            return std::exp(std::log(lo) * (1.f - t) + std::log(hi) * t);
        return lo * (1.f - t) + hi * t;
    };

    float v = 0.f;
    if      (LADSPA_IS_HINT_DEFAULT_0(h.HintDescriptor))   v = apply_sr_hint(0.f,   h, sr);
    else if (LADSPA_IS_HINT_DEFAULT_1(h.HintDescriptor))   v = apply_sr_hint(1.f,   h, sr);
    else if (LADSPA_IS_HINT_DEFAULT_100(h.HintDescriptor)) v = apply_sr_hint(100.f, h, sr);
    else if (LADSPA_IS_HINT_DEFAULT_440(h.HintDescriptor)) v = apply_sr_hint(440.f, h, sr);
    else if (LADSPA_IS_HINT_DEFAULT_MINIMUM(h.HintDescriptor)) v = lo;
    else if (LADSPA_IS_HINT_DEFAULT_MAXIMUM(h.HintDescriptor)) v = hi;
    else if (LADSPA_IS_HINT_DEFAULT_LOW(h.HintDescriptor))    v = lerp(0.25f);
    else if (LADSPA_IS_HINT_DEFAULT_MIDDLE(h.HintDescriptor)) v = lerp(0.50f);
    else if (LADSPA_IS_HINT_DEFAULT_HIGH(h.HintDescriptor))   v = lerp(0.75f);

    if (LADSPA_IS_HINT_BOUNDED_BELOW(h.HintDescriptor)) v = std::max(v, lo);
    if (LADSPA_IS_HINT_BOUNDED_ABOVE(h.HintDescriptor)) v = std::min(v, hi);
    return v;
}

// ─── Main ─────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    if (argc != 5) {
        fprintf(stdout, "ERROR usage: dssi_sandbox <shm_name> <path> <index> <srate>\n");
        fflush(stdout);
        return 1;
    }

    const char* shm_name   = argv[1];
    const char* plugin_path = argv[2];
    int         plugin_idx  = atoi(argv[3]);
    double      sample_rate = atof(argv[4]);

    // ── 1. Attach to shared memory (created by parent) ─────────────────
    int shm_fd = shm_open(shm_name, O_RDWR, 0600);
    if (shm_fd == -1) {
        fprintf(stdout, "ERROR shm_open(%s): %s\n", shm_name, strerror(errno));
        fflush(stdout);
        return 1;
    }

    void* shm_raw = mmap(nullptr, DSB_SHM_SIZE, PROT_READ | PROT_WRITE,
                         MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    if (shm_raw == MAP_FAILED) {
        fprintf(stdout, "ERROR mmap: %s\n", strerror(errno));
        fflush(stdout);
        return 1;
    }
    DSSIBridgeShm* shm = static_cast<DSSIBridgeShm*>(shm_raw);

    // ── 2. Load DSSI plugin ────────────────────────────────────────────
    void* lib = dlopen(plugin_path, RTLD_NOW);
    if (!lib) {
        fprintf(stdout, "ERROR dlopen: %s\n", dlerror());
        fflush(stdout);
        return 1;
    }

    DSSI_Descriptor_Function df =
        (DSSI_Descriptor_Function)dlsym(lib, "dssi_descriptor");
    if (!df) {
        fprintf(stdout, "ERROR missing dssi_descriptor symbol\n");
        fflush(stdout);
        dlclose(lib);
        return 1;
    }

    const DSSI_Descriptor* desc = df((unsigned long)plugin_idx);
    if (!desc) {
        fprintf(stdout, "ERROR dssi_descriptor(%d) returned null\n", plugin_idx);
        fflush(stdout);
        dlclose(lib);
        return 1;
    }

    const LADSPA_Descriptor* ladspa = desc->LADSPA_Plugin;
    LADSPA_Handle inst = ladspa->instantiate(ladspa, (unsigned long)sample_rate);
    if (!inst) {
        fprintf(stdout, "ERROR plugin instantiate failed\n");
        fflush(stdout);
        dlclose(lib);
        return 1;
    }

    // ── 3. Discover ports ─────────────────────────────────────────────
    struct CtrlPort { int port_idx; float min_v, max_v, def_v; std::string name; };
    std::vector<CtrlPort> ctrl_ports;
    std::vector<float>    port_values(ladspa->PortCount, 0.f);
    std::vector<float>    dummy_buf(DSB_MAX_FRAMES, 0.f);
    int audio_out_l = -1, audio_out_r = -1;

    for (unsigned long i = 0; i < ladspa->PortCount; ++i) {
        LADSPA_PortDescriptor d = ladspa->PortDescriptors[i];
        if (LADSPA_IS_PORT_AUDIO(d)) {
            if (LADSPA_IS_PORT_OUTPUT(d)) {
                if      (audio_out_l == -1) audio_out_l = (int)i;
                else if (audio_out_r == -1) audio_out_r = (int)i;
                else ladspa->connect_port(inst, i, dummy_buf.data());
            } else {
                ladspa->connect_port(inst, i, dummy_buf.data());
            }
        } else if (LADSPA_IS_PORT_CONTROL(d)) {
            if (LADSPA_IS_PORT_INPUT(d)) {
                const auto& h = ladspa->PortRangeHints[i];
                float lo  = LADSPA_IS_HINT_BOUNDED_BELOW(h.HintDescriptor) ? apply_sr_hint(h.LowerBound, h, sample_rate) : 0.f;
                float hi  = LADSPA_IS_HINT_BOUNDED_ABOVE(h.HintDescriptor) ? apply_sr_hint(h.UpperBound, h, sample_rate) : 1.f;
                float def = LADSPA_IS_HINT_HAS_DEFAULT(h.HintDescriptor) ? default_port_value(h, sample_rate) : 0.f;
                if (hi <= lo) hi = lo + 1.f;
                port_values[i] = def;
                ladspa->connect_port(inst, i, &port_values[i]);
                ctrl_ports.push_back({(int)i, lo, hi, def, ladspa->PortNames[i]});
            } else {
                ladspa->connect_port(inst, i, &port_values[i]);
            }
        }
    }

    if (ladspa->activate) ladspa->activate(inst);

    // Pre-reserve MIDI buffer
    std::vector<snd_seq_event_t> midi_evs;
    midi_evs.reserve(DSB_MAX_MIDI);

    // ── 4. Report setup to parent ──────────────────────────────────────
    fprintf(stdout, "OK\n");
    fprintf(stdout, "NAME %s\n", ladspa->Name ? ladspa->Name : "");
    fprintf(stdout, "HAS_SYNTH %d\n", desc->run_synth ? 1 : 0);
    fprintf(stdout, "CTRL_COUNT %zu\n", ctrl_ports.size());
    for (size_t i = 0; i < ctrl_ports.size(); ++i) {
        const auto& c = ctrl_ports[i];
        // Escape spaces in name with \x1F (unlikely to appear in port names)
        std::string safe_name = c.name;
        for (char& ch : safe_name) if (ch == '\n') ch = ' ';
        fprintf(stdout, "CTRL %zu %d %.8g %.8g %.8g %s\n",
                i, c.port_idx, c.min_v, c.max_v, c.def_v, safe_name.c_str());
    }
    fprintf(stdout, "AUDIO_L %d\n", audio_out_l);
    fprintf(stdout, "AUDIO_R %d\n", audio_out_r);
    fprintf(stdout, "READY\n");
    fflush(stdout);

    // ── 5. RT loop ────────────────────────────────────────────────────
    // Copy initial port values from shm (host may have set them during setup).
    for (size_t i = 0; i < ctrl_ports.size() && i < DSB_MAX_PORTS; ++i)
        port_values[(size_t)ctrl_ports[i].port_idx] = shm->port_values[i];

    while (true) {
        if (sem_wait(&shm->sem_req) == -1) {
            if (errno == EINTR) continue;
            break;
        }

        if (shm->shutdown) break;

        uint32_t nf = shm->nframes;
        if (nf == 0 || nf > DSB_MAX_FRAMES) nf = 0;

        // Update control port values from shm
        uint32_t pcnt = shm->port_count < DSB_MAX_PORTS ? shm->port_count : DSB_MAX_PORTS;
        for (uint32_t i = 0; i < pcnt && i < ctrl_ports.size(); ++i)
            port_values[(size_t)ctrl_ports[i].port_idx] = shm->port_values[i];

        // Handle deferred program change
        if (shm->select_program_pending && desc->select_program) {
            desc->select_program(inst,
                                 shm->select_program_bank,
                                 shm->select_program_program);
            shm->select_program_pending = 0;
        }

        // Decode MIDI events
        midi_evs.clear();
        uint32_t mc = shm->midi_count < DSB_MAX_MIDI ? shm->midi_count : DSB_MAX_MIDI;
        for (uint32_t i = 0; i < mc; ++i) {
            const DsbMidiEvent& me = shm->midi_events[i];
            snd_seq_event_t ev;
            memset(&ev, 0, sizeof(ev));
            if (me.type == 0x90) {
                ev.type = SND_SEQ_EVENT_NOTEON;
                ev.data.note.channel  = me.channel;
                ev.data.note.note     = me.note;
                ev.data.note.velocity = me.value;
            } else if (me.type == 0x80) {
                ev.type = SND_SEQ_EVENT_NOTEOFF;
                ev.data.note.channel  = me.channel;
                ev.data.note.note     = me.note;
                ev.data.note.velocity = 0;
            } else if (me.type == 0xB0) {
                ev.type = SND_SEQ_EVENT_CONTROLLER;
                ev.data.control.channel = me.channel;
                ev.data.control.param   = me.note;
                ev.data.control.value   = me.value;
            } else {
                continue;
            }
            midi_evs.push_back(ev);
        }

        // Connect audio output ports to shm buffers
        if (audio_out_l != -1)
            ladspa->connect_port(inst, (unsigned long)audio_out_l, shm->audio_l);
        else
            for (uint32_t i = 0; i < nf; ++i) shm->audio_l[i] = 0.f;

        if (audio_out_r != -1)
            ladspa->connect_port(inst, (unsigned long)audio_out_r, shm->audio_r);
        else
            for (uint32_t i = 0; i < nf; ++i) shm->audio_r[i] = 0.f;

        if (nf > 0) {
            if (desc->run_synth)
                desc->run_synth(inst, nf, midi_evs.data(), (unsigned long)midi_evs.size());
            else
                ladspa->run(inst, nf);

            // Mono → stereo copy
            if (audio_out_l != -1 && audio_out_r == -1)
                for (uint32_t i = 0; i < nf; ++i) shm->audio_r[i] = shm->audio_l[i];
        }

        sem_post(&shm->sem_done);
    }

    // ── 6. Cleanup ────────────────────────────────────────────────────
    if (ladspa->deactivate) ladspa->deactivate(inst);
    ladspa->cleanup(inst);
    dlclose(lib);
    munmap(shm_raw, DSB_SHM_SIZE);
    return 0;
}
