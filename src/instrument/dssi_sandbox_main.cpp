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
 *     PROGRAM_COUNT <n>\n
 *     PROGRAM <seq_idx> <bank> <program> <name>\n  (×n)
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
#include <cctype>
#include <cmath>
#include <cerrno>
#include <algorithm>
#include <string>
#include <vector>
#include <filesystem>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

using namespace disgrace_ns;
namespace fs = std::filesystem;

// ─── Helpers ──────────────────────────────────────────────────────────────────

static float apply_sr_hint(float v, const LADSPA_PortRangeHint& h, double sr)
{
    return LADSPA_IS_HINT_SAMPLE_RATE(h.HintDescriptor) ? v * (float)sr : v;
}

static std::string ascii_lower(std::string s)
{
    for (char& c : s)
        c = (char)std::tolower((unsigned char)c);
    return s;
}

// True when a port name identifies a channel side, e.g. "Output L",
// "Left", "out_r" or "Main Right". Matching is token based so that names
// such as "Level" or "Release" do not accidentally match a bare L/R.
static bool port_name_is_side(const char* raw, char side)
{
    if (!raw || !*raw) return false;
    const std::string n = ascii_lower(raw);
    const char* want_word = (side == 'l') ? "left" : "right";
    std::string token;
    for (size_t i = 0; i <= n.size(); ++i) {
        const char c = (i < n.size()) ? n[i] : '\0';
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
            token.push_back(c);
        } else if (!token.empty()) {
            if (token == want_word) return true;
            if (token.size() == 1 && token[0] == side) return true;
            token.clear();
        }
    }
    return false;
}

static const char* safe_port_name(const LADSPA_Descriptor* d, int port)
{
    if (!d || port < 0 || (unsigned long)port >= d->PortCount) return "";
    const char* n = d->PortNames ? d->PortNames[port] : nullptr;
    return n ? n : "";
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

static bool chdir_to_degenerat_preset_root(const char* plugin_path)
{
    if (!plugin_path || std::string(plugin_path).find("degenerat") == std::string::npos) {
        return false;
    }

    fs::path probe = fs::current_path();
    for (int depth = 0; depth < 4 && !probe.empty(); ++depth, probe = probe.parent_path()) {
        std::error_code ec;
        for (fs::directory_iterator it(probe, ec), end; !ec && it != end; it.increment(ec)) {
            const auto entry_name = it->path().filename().string();
            if (entry_name != "degenerat") {
                continue;
            }

            fs::path preset_root = it->path() / "data" / "presets";
            if (!fs::is_directory(preset_root, ec)) {
                continue;
            }

            fs::current_path(it->path(), ec);
            if (!ec) {
                return true;
            }
        }
    }

    return false;
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

    // The host closes the setup pipe once it has read READY. A plugin (or a
    // library underneath it) writing to stdout must never kill the sandbox.
    signal(SIGPIPE, SIG_IGN);

    (void)chdir_to_degenerat_preset_root(plugin_path);

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

    // Every audio port that is not the stereo output gets its own zeroed
    // scratch buffer. Sharing one buffer between audio inputs and surplus
    // outputs lets a multi-output plugin read its own output back (feedback)
    // or write into a buffer another port is reading.
    std::vector<std::vector<float>> scratch_bufs;
    auto new_scratch = [&]() -> float* {
        scratch_bufs.emplace_back(DSB_MAX_FRAMES, 0.f);
        return scratch_bufs.back().data();
    };

    std::vector<int> audio_outs;
    for (unsigned long i = 0; i < ladspa->PortCount; ++i) {
        LADSPA_PortDescriptor d = ladspa->PortDescriptors[i];
        if (LADSPA_IS_PORT_AUDIO(d)) {
            ladspa->connect_port(inst, i, new_scratch());
            if (LADSPA_IS_PORT_OUTPUT(d)) audio_outs.push_back((int)i);
        } else if (LADSPA_IS_PORT_CONTROL(d)) {
            if (LADSPA_IS_PORT_INPUT(d)) {
                const auto& h = ladspa->PortRangeHints[i];
                float lo  = LADSPA_IS_HINT_BOUNDED_BELOW(h.HintDescriptor) ? apply_sr_hint(h.LowerBound, h, sample_rate) : 0.f;
                float hi  = LADSPA_IS_HINT_BOUNDED_ABOVE(h.HintDescriptor) ? apply_sr_hint(h.UpperBound, h, sample_rate) : 1.f;
                float def = LADSPA_IS_HINT_HAS_DEFAULT(h.HintDescriptor) ? default_port_value(h, sample_rate) : 0.f;
                if (hi <= lo) hi = lo + 1.f;
                port_values[i] = def;
                ladspa->connect_port(inst, i, &port_values[i]);
                ctrl_ports.push_back({(int)i, lo, hi, def,
                                      ladspa->PortNames && ladspa->PortNames[i] ? ladspa->PortNames[i] : ""});
            } else {
                ladspa->connect_port(inst, i, &port_values[i]);
            }
        }
    }

    // Pick the stereo pair. Prefer ports whose names identify the side
    // ("Output L"/"Right"/...): plugins are free to put their two outputs
    // anywhere in the port list (karplong has L at 0 and R at 5), and a
    // multi-output plugin's first two audio ports are not necessarily the
    // main stereo pair. Fall back to the first two audio outputs.
    int audio_out_l = -1, audio_out_r = -1;
    if (!audio_outs.empty()) {
        int named_l = -1, named_r = -1;
        for (int p : audio_outs) {
            const char* n = safe_port_name(ladspa, p);
            if (named_l == -1 && port_name_is_side(n, 'l')) { named_l = p; continue; }
            if (named_r == -1 && port_name_is_side(n, 'r')) { named_r = p; }
        }
        if (named_l != -1 && named_r != -1) {
            audio_out_l = named_l;
            audio_out_r = named_r;
        } else if (named_l != -1 && audio_outs.size() >= 2) {
            audio_out_l = named_l;
            audio_out_r = (audio_outs[0] == named_l) ? audio_outs[1] : audio_outs[0];
        } else if (named_r != -1 && audio_outs.size() >= 2) {
            audio_out_r = named_r;
            audio_out_l = (audio_outs[0] == named_r) ? audio_outs[1] : audio_outs[0];
        } else if (audio_outs.size() >= 2) {
            audio_out_l = audio_outs[0];
            audio_out_r = audio_outs[1];
        } else {
            audio_out_l = audio_outs[0]; // mono
        }
    }

    if (audio_out_l == -1) {
        // Nothing to mix into the host: fail loudly instead of loading a
        // "working" instrument that can only ever produce silence.
        fprintf(stdout, "ERROR plugin has no audio output ports\n");
        fflush(stdout);
        ladspa->cleanup(inst);
        dlclose(lib);
        munmap(shm_raw, DSB_SHM_SIZE);
        return 1;
    }

    // Connect the two mix outputs and clear them before activate() so the
    // plugin never sees dangling port pointers.
    memset(shm->audio_l, 0, sizeof(shm->audio_l));
    memset(shm->audio_r, 0, sizeof(shm->audio_r));
    ladspa->connect_port(inst, (unsigned long)audio_out_l, shm->audio_l);
    if (audio_out_r != -1)
        ladspa->connect_port(inst, (unsigned long)audio_out_r, shm->audio_r);

    fprintf(stderr,
            "[dssi_sandbox] %s: %.0f Hz, audio out L=%d(%s) R=%d(%s), %zu audio outs, %zu controls\n",
            ladspa->Name ? ladspa->Name : "?",
            sample_rate,
            audio_out_l, safe_port_name(ladspa, audio_out_l),
            audio_out_r, safe_port_name(ladspa, audio_out_r),
            audio_outs.size(), ctrl_ports.size());

    if (ladspa->activate) ladspa->activate(inst);

    // Pre-reserve MIDI buffer
    std::vector<snd_seq_event_t> midi_evs;
    midi_evs.reserve(DSB_MAX_MIDI);

    struct ProgramInfo {
        unsigned long bank = 0;
        unsigned long program = 0;
        std::string name;
    };
    std::vector<ProgramInfo> programs;
    if (desc->get_program) {
        for (unsigned long i = 0; ; ++i) {
            const DSSI_Program_Descriptor* prog = desc->get_program(inst, i);
            if (!prog) break;
            programs.push_back({
                prog->Bank,
                prog->Program,
                prog->Name ? prog->Name : ""
            });
        }
    }

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
    fprintf(stdout, "PROGRAM_COUNT %zu\n", programs.size());
    for (size_t i = 0; i < programs.size(); ++i) {
        fprintf(stdout, "PROGRAM %zu %lu %lu %s\n",
                i, programs[i].bank, programs[i].program, programs[i].name.c_str());
    }
    fprintf(stdout, "AUDIO_L %d\n", audio_out_l);
    fprintf(stdout, "AUDIO_R %d\n", audio_out_r);
    fprintf(stdout, "READY\n");
    fflush(stdout);

    // ── 4b. Detach stdout from the host pipe ──────────────────────────
    // The host closes its read end once the handshake is complete. Point
    // stdout at /dev/null: a plugin that printf()s from run_synth() would
    // otherwise fill the pipe buffer and block inside write(), freezing the
    // RT loop. That looked exactly like a dead plugin and was undetectable.
    {
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            dup2(devnull, STDOUT_FILENO);
            close(devnull);
        }
    }

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

        // Output ports are connected once, before activate().  The plugin
        // writes straight into the shared stereo buffers, so the host mixes
        // exactly what the plugin produced through the track's channel.
        if (nf > 0) {
            if (desc->run_synth)
                desc->run_synth(inst, nf, midi_evs.data(), (unsigned long)midi_evs.size());
            else
                ladspa->run(inst, nf);

            // Mono → stereo copy
            if (audio_out_r == -1)
                for (uint32_t i = 0; i < nf; ++i) shm->audio_r[i] = shm->audio_l[i];
        } else {
            // Nothing rendered: keep the host from mixing a stale block.
            memset(shm->audio_l, 0, sizeof(float) * DSB_MAX_FRAMES);
            memset(shm->audio_r, 0, sizeof(float) * DSB_MAX_FRAMES);
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
