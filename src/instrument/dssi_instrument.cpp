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

#include "dssi_instrument.h"

#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cerrno>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

namespace disgrace_ns {

// ─── Helpers ──────────────────────────────────────────────────────────────────

std::string DSSIInstrument::find_sandbox_binary()
{
    char exe_buf[4096] = {};
    ssize_t n = readlink("/proc/self/exe", exe_buf, sizeof(exe_buf) - 1);
    if (n > 0) {
        std::string exe(exe_buf, (size_t)n);
        auto slash = exe.rfind('/');
        if (slash != std::string::npos) {
            std::string candidate = exe.substr(0, slash + 1) + "dssi_sandbox";
            if (access(candidate.c_str(), X_OK) == 0)
                return candidate;
        }
    }
    return "dssi_sandbox";
}

// ─── Constructor / destructor ─────────────────────────────────────────────────

DSSIInstrument::DSSIInstrument(double sample_rate) : m_sample_rate(sample_rate) {}

DSSIInstrument::~DSSIInstrument()
{
    teardown_sandbox();
}

// ─── Sandbox lifecycle ────────────────────────────────────────────────────────

void DSSIInstrument::teardown_sandbox()
{
    m_alive.store(false);

    if (m_child_pid > 0 && m_shm_ptr) {
        shm()->shutdown = 1;
        sem_post(&shm()->sem_req);
        for (int i = 0; i < 20; ++i) {
            usleep(10000);
            int status;
            if (waitpid(m_child_pid, &status, WNOHANG) == m_child_pid)
                goto cleanup;
        }
        kill(m_child_pid, SIGKILL);
        waitpid(m_child_pid, nullptr, 0);
    }
cleanup:
    m_child_pid = -1;

    if (m_stdout_rfd >= 0) {
        close(m_stdout_rfd);
        m_stdout_rfd = -1;
    }

    if (m_shm_ptr && m_shm_ptr != MAP_FAILED) {
        sem_destroy(&shm()->sem_req);
        sem_destroy(&shm()->sem_done);
        munmap(m_shm_ptr, DSB_SHM_SIZE);
        m_shm_ptr = nullptr;
    }
    if (!m_shm_name.empty()) {
        shm_unlink(m_shm_name.c_str());
        m_shm_name.clear();
    }
}

bool DSSIInstrument::spawn_sandbox(const std::string& plugin_path, int plugin_idx)
{
    teardown_sandbox();

    // ── Shared memory ────────────────────────────────────────────────
    char shm_name_buf[64];
    static int s_seq = 0;
    snprintf(shm_name_buf, sizeof(shm_name_buf),
             "/dgr_dssi_%d_%d", (int)getpid(), s_seq++);
    m_shm_name = shm_name_buf;

    shm_unlink(m_shm_name.c_str());
    int shm_fd = shm_open(m_shm_name.c_str(), O_CREAT | O_RDWR, 0600);
    if (shm_fd == -1) {
        std::cerr << "[DSSIInstrument] shm_open: " << strerror(errno) << "\n";
        return false;
    }
    if (ftruncate(shm_fd, (off_t)DSB_SHM_SIZE) == -1) {
        std::cerr << "[DSSIInstrument] ftruncate: " << strerror(errno) << "\n";
        close(shm_fd);
        shm_unlink(m_shm_name.c_str());
        return false;
    }

    void* raw = mmap(nullptr, DSB_SHM_SIZE, PROT_READ | PROT_WRITE,
                     MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    if (raw == MAP_FAILED) {
        std::cerr << "[DSSIInstrument] mmap: " << strerror(errno) << "\n";
        shm_unlink(m_shm_name.c_str());
        return false;
    }
    m_shm_ptr = raw;
    memset(raw, 0, DSB_SHM_SIZE);

    if (sem_init(&shm()->sem_req,  1, 0) == -1 ||
        sem_init(&shm()->sem_done, 1, 0) == -1) {
        std::cerr << "[DSSIInstrument] sem_init: " << strerror(errno) << "\n";
        teardown_sandbox();
        return false;
    }

    // ── Pipe for child stdout (setup protocol) ────────────────────────
    int pfd[2];
    if (pipe(pfd) == -1) {
        std::cerr << "[DSSIInstrument] pipe: " << strerror(errno) << "\n";
        teardown_sandbox();
        return false;
    }

    // ── Fork ──────────────────────────────────────────────────────────
    std::string sandbox_bin = find_sandbox_binary();
    char sr_buf[32];
    snprintf(sr_buf, sizeof(sr_buf), "%.0f", m_sample_rate);
    char idx_buf[16];
    snprintf(idx_buf, sizeof(idx_buf), "%d", plugin_idx);

    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "[DSSIInstrument] fork: " << strerror(errno) << "\n";
        close(pfd[0]); close(pfd[1]);
        teardown_sandbox();
        return false;
    }

    if (pid == 0) {
        // Child: redirect stdout → write end of pipe, then exec
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        close(pfd[1]);
        int maxfd = (int)sysconf(_SC_OPEN_MAX);
        for (int i = 3; i < maxfd; ++i) close(i);

        char* args[] = {
            const_cast<char*>(sandbox_bin.c_str()),
            const_cast<char*>(m_shm_name.c_str()),
            const_cast<char*>(plugin_path.c_str()),
            idx_buf,
            sr_buf,
            nullptr
        };
        execvp(sandbox_bin.c_str(), args);
        dprintf(STDOUT_FILENO, "ERROR execvp(%s): %s\n",
                sandbox_bin.c_str(), strerror(errno));
        _exit(127);
    }

    // Parent
    close(pfd[1]);
    m_child_pid = pid;

    // ── Read setup protocol from child ────────────────────────────────
    FILE* f = fdopen(pfd[0], "r");
    if (!f) {
        std::cerr << "[DSSIInstrument] fdopen failed\n";
        teardown_sandbox();
        return false;
    }

    bool ok = false;
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = '\0';

        if (strncmp(line, "ERROR ", 6) == 0) {
            std::cerr << "[DSSIInstrument] child: " << line << "\n";
            break;
        } else if (strcmp(line, "OK") == 0) {
            ok = true;
        } else if (strncmp(line, "NAME ", 5) == 0) {
            set_plugin_name(line + 5);
        } else if (strncmp(line, "HAS_SYNTH ", 10) == 0) {
            m_has_run_synth = (line[10] == '1');
        } else if (strncmp(line, "CTRL_COUNT ", 11) == 0) {
            int cnt = atoi(line + 11);
            m_ctrl_info.clear();
            m_ctrl_info.reserve((size_t)cnt);
        } else if (strncmp(line, "CTRL ", 5) == 0) {
            CtrlInfo ci{};
            char name_buf[512] = {};
            int seq_idx = 0;
            if (sscanf(line + 5, "%d %d %f %f %f %511[^\n]",
                       &seq_idx, &ci.port_idx,
                       &ci.min_v, &ci.max_v, &ci.def_v, name_buf) >= 5) {
                ci.name = name_buf;
                ci.current_value = ci.def_v;
                m_ctrl_info.push_back(ci);
            }
        } else if (strncmp(line, "AUDIO_L ", 8) == 0) {
            m_audio_out_l = atoi(line + 8);
        } else if (strncmp(line, "AUDIO_R ", 8) == 0) {
            m_audio_out_r = atoi(line + 8);
        } else if (strcmp(line, "READY") == 0) {
            break;
        }
    }

    // Retain the raw fd; detach the FILE wrapper without closing the underlying fd.
    int tmp_fd = dup(fileno(f));
    fclose(f);
    m_stdout_rfd = tmp_fd;

    if (!ok) {
        std::cerr << "[DSSIInstrument] plugin failed to start\n";
        teardown_sandbox();
        return false;
    }

    // Populate shm port values with defaults
    shm()->port_count = (uint32_t)m_ctrl_info.size();
    for (size_t i = 0; i < m_ctrl_info.size() && i < DSB_MAX_PORTS; ++i)
        shm()->port_values[i] = m_ctrl_info[i].current_value;

    m_pending_midi.reserve(DSB_MAX_MIDI);

    // Apply any saved program selection
    if (m_bank != 0 || m_program != 0) {
        shm()->select_program_bank    = (uint32_t)m_bank;
        shm()->select_program_program = (uint32_t)m_program;
        shm()->select_program_pending = 1;
    }

    m_alive.store(true);
    return true;
}

// ─── Public API ───────────────────────────────────────────────────────────────

bool DSSIInstrument::load_plugin(const std::string& path, int index)
{
    m_path  = path;
    m_index = index;
    m_ctrl_info.clear();
    m_bank    = 0;
    m_program = 0;
    return spawn_sandbox(path, index);
}

bool DSSIInstrument::is_alive() const
{
    if (!m_alive.load()) return false;
    if (m_child_pid > 0 && kill(m_child_pid, 0) == -1 && errno == ESRCH) {
        m_alive.store(false);
        return false;
    }
    return true;
}

Instrument::Parameter DSSIInstrument::get_parameter(size_t index) const
{
    if (index >= m_ctrl_info.size()) return {};
    const auto& ci = m_ctrl_info[index];
    Instrument::Parameter p;
    p.index = ci.port_idx;
    p.name  = ci.name;
    p.min   = ci.min_v;
    p.max   = ci.max_v;
    p.value = ci.current_value;
    return p;
}

void DSSIInstrument::set_parameter(size_t index, float value)
{
    if (index >= m_ctrl_info.size()) return;
    m_ctrl_info[index].current_value = value;
    if (m_shm_ptr && index < DSB_MAX_PORTS)
        shm()->port_values[index] = value;
}

void DSSIInstrument::load_program(unsigned long bank, unsigned long program)
{
    m_bank    = bank;
    m_program = program;
    if (m_shm_ptr && is_alive()) {
        shm()->select_program_bank    = (uint32_t)bank;
        shm()->select_program_program = (uint32_t)program;
        shm()->select_program_pending = 1;
    }
}

void DSSIInstrument::set_volume(float vol) { m_volume = std::max(0.0f, vol); }
void DSSIInstrument::set_pitch(float) {}

void DSSIInstrument::note_on(uint8_t note, uint8_t velocity,
                              size_t column_index, size_t, uint8_t)
{
    const size_t chan = column_index % 16;
    if (m_last_note[chan] != -1) note_off(column_index);
    m_pending_midi.push_back({0x90, (uint8_t)chan, note, velocity});
    m_last_note[chan] = note;
}

void DSSIInstrument::note_off(size_t column_index)
{
    const size_t chan = column_index % 16;
    if (m_last_note[chan] == -1) return;
    m_pending_midi.push_back({0x80, (uint8_t)chan, (uint8_t)m_last_note[chan], 0});
    m_last_note[chan] = -1;
}

void DSSIInstrument::panic()
{
    for (size_t chan = 0; chan < 16; ++chan) {
        m_pending_midi.push_back({0xB0, (uint8_t)chan, 123, 0});
        m_last_note[chan] = -1;
    }
}

void DSSIInstrument::process(float* l, float* r, size_t nframes)
{
    if (nframes > DSB_MAX_FRAMES) nframes = DSB_MAX_FRAMES;
    memset(l, 0, nframes * sizeof(float));
    memset(r, 0, nframes * sizeof(float));

    if (!is_alive()) {
        m_pending_midi.clear();
        return;
    }

    DSSIBridgeShm* s = shm();

    s->nframes    = (uint32_t)nframes;
    s->volume     = m_volume;
    s->port_count = (uint32_t)m_ctrl_info.size();
    for (size_t i = 0; i < m_ctrl_info.size() && i < DSB_MAX_PORTS; ++i)
        s->port_values[i] = m_ctrl_info[i].current_value;

    uint32_t mc = (uint32_t)std::min(m_pending_midi.size(), (size_t)DSB_MAX_MIDI);
    s->midi_count = mc;
    for (uint32_t i = 0; i < mc; ++i) {
        const auto& me = m_pending_midi[i];
        s->midi_events[i] = {me.type, me.channel, me.note, me.value};
    }
    m_pending_midi.clear();
    s->shutdown = 0;

    sem_post(&s->sem_req);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += 10000000L; // 10 ms deadline
    if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }

    if (sem_timedwait(&s->sem_done, &ts) == -1) {
        if (!is_alive())
            std::cerr << "[DSSIInstrument] sandbox died — silenced\n";
        return;
    }

    memcpy(l, s->audio_l, nframes * sizeof(float));
    memcpy(r, s->audio_r, nframes * sizeof(float));

    for (size_t i = 0; i < nframes; ++i) {
        l[i] *= m_volume;
        r[i] *= m_volume;
    }
}

} // namespace disgrace_ns
