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
#include <poll.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#if defined(__FreeBSD__) || defined(__DragonFly__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

namespace disgrace_ns {

// ─── Helpers ──────────────────────────────────────────────────────────────────

// How long the parent waits for the child's setup protocol (OK…READY).
// Generous: large sample-based synths can take a while to instantiate.
static constexpr int kSetupTimeoutMs = 15000;

static int64_t monotonic_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// Resolve the absolute path of the running executable.  Portable across the
// platforms Disgrace supports:
//   Linux   → /proc/self/exe
//   FreeBSD → kern.proc.pathname (readlink("/proc/self/exe") does not exist
//             unless the native procfs is mounted, which it usually is not)
// Returns an empty string if the path cannot be determined.
static std::string current_executable_path()
{
#if defined(__linux__)
    char buf[4096] = {};
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n > 0)
        return std::string(buf, (size_t)n);
#elif defined(__FreeBSD__) || defined(__DragonFly__)
    int    mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, (int)getpid() };
    char   buf[4096] = {};
    size_t len = sizeof(buf);
    if (sysctl(mib, 4, buf, &len, nullptr, 0) == 0 && len > 0)
        return std::string(buf);
#endif
    return {};
}

std::string DSSIInstrument::find_sandbox_binary()
{
    // The dssi_sandbox helper is installed next to the main executable, so
    // derive its location from the running binary rather than relying on
    // $PATH (the shell PATH is not guaranteed to contain the install dir).
    std::string exe = current_executable_path();
    if (!exe.empty()) {
        auto slash = exe.rfind('/');
        if (slash != std::string::npos) {
            std::string candidate = exe.substr(0, slash + 1) + "dssi_sandbox";
            if (access(candidate.c_str(), X_OK) == 0)
                return candidate;
        }
    }
    // Last resort: let execvp() search $PATH for the bare name.
    return "dssi_sandbox";
}

// ─── Constructor / destructor ─────────────────────────────────────────────────

DSSIInstrument::DSSIInstrument(double sample_rate)
    : m_sample_rate(sample_rate > 0.0 ? (uint32_t)(sample_rate + 0.5) : 44100u) {}

DSSIInstrument::~DSSIInstrument()
{
    teardown_sandbox();
}

// ─── Sandbox lifecycle ────────────────────────────────────────────────────────

void DSSIInstrument::teardown_sandbox()
{
    m_alive.store(false);

    // Take ownership of the pid. After this exchange the RT thread's liveness
    // probe can no longer touch it, and the pid is never signalled twice —
    // signalling a reaped pid would risk hitting a recycled, unrelated process.
    const pid_t pid = m_child_pid.exchange(-1, std::memory_order_acq_rel);

    if (pid > 0) {
        if (m_shm_ptr) {
            shm()->shutdown = 1;
            sem_post(&shm()->sem_req);
        }
        bool reaped = false;
        for (int i = 0; i < 20 && m_shm_ptr; ++i) {
            usleep(10000);
            int status = 0;
            const pid_t r = waitpid(pid, &status, WNOHANG);
            if (r == pid) { reaped = true; break; }
            // ECHILD: someone else (the liveness probe) already reaped it.
            if (r == -1 && errno == ECHILD) { reaped = true; break; }
        }
        if (!reaped) {
            kill(pid, SIGKILL);
            waitpid(pid, nullptr, 0);
        }
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

    // Per-session runtime state must not survive into the next spawn.
    m_request_pending = false;
    m_pending_frames  = 0;
    m_pending_midi.clear();
    m_wait_samples    = 0;
    fifo_reset();
    for (auto& n : m_last_note) n = -1;
}

bool DSSIInstrument::spawn_sandbox(const std::string& plugin_path, int plugin_idx)
{
    teardown_sandbox();
    m_audio_out_l = -1;
    m_audio_out_r = -1;
    m_has_run_synth = false;

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
    snprintf(sr_buf, sizeof(sr_buf), "%u",
             (unsigned)m_sample_rate.load(std::memory_order_relaxed));
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
        // The host closes the read end once the handshake is done and the
        // child then points stdout at /dev/null. Ignore SIGPIPE as well so
        // that a stray write can never kill the sandbox mid-song.
        signal(SIGPIPE, SIG_IGN);
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
    m_child_pid.store(pid, std::memory_order_relaxed);

    // ── Read setup protocol from child (bounded in time) ──────────────
    // This runs on the GUI thread, so a plugin that hangs in instantiate()
    // must not block it forever: poll with a deadline instead of fgets().
    const int64_t deadline = monotonic_ms() + kSetupTimeoutMs;
    std::string rx;          // received bytes not yet split into lines
    bool timed_out = false;

    auto pump = [&]() -> bool {
        const int64_t left = deadline - monotonic_ms();
        if (left <= 0) { timed_out = true; return false; }
        struct pollfd p{ pfd[0], POLLIN, 0 };
        const int pr = poll(&p, 1, (int)std::min<int64_t>(left, 250));
        if (pr == 0) return true;                       // re-check the deadline
        if (pr < 0) return errno == EINTR;              // else: error → stop
        char buf[512];
        const ssize_t n = read(pfd[0], buf, sizeof(buf));
        if (n > 0) { rx.append(buf, (size_t)n); return true; }
        if (n < 0 && errno == EINTR) return true;
        return false;                                   // EOF or fatal read error
    };

    auto next_line = [&](std::string& out) -> bool {
        for (;;) {
            const size_t nl = rx.find('\n');
            if (nl != std::string::npos) {
                out.assign(rx, 0, nl);
                rx.erase(0, nl + 1);
                while (!out.empty() && out.back() == '\r') out.pop_back();
                return true;
            }
            if (!pump()) return false;
        }
    };

    bool ok = false;
    std::string err_line;
    std::string line;
    while (next_line(line)) {
        const char* l = line.c_str();

        if (strncmp(l, "ERROR ", 6) == 0) {
            err_line = l + 6;
            break;
        } else if (strcmp(l, "OK") == 0) {
            ok = true;
        } else if (strncmp(l, "NAME ", 5) == 0) {
            set_plugin_name(l + 5);
        } else if (strncmp(l, "HAS_SYNTH ", 10) == 0) {
            m_has_run_synth = (l[10] == '1');
        } else if (strncmp(l, "CTRL_COUNT ", 11) == 0) {
            int cnt = atoi(l + 11);
            m_ctrl_info.clear();
            m_ctrl_info.reserve((size_t)cnt);
        } else if (strncmp(l, "CTRL ", 5) == 0) {
            CtrlInfo ci{};
            char name_buf[512] = {};
            int seq_idx = 0;
            if (sscanf(l + 5, "%d %d %f %f %f %511[^\n]",
                       &seq_idx, &ci.port_idx,
                       &ci.min_v, &ci.max_v, &ci.def_v, name_buf) >= 5) {
                ci.name = name_buf;
                ci.current_value = ci.def_v;
                m_ctrl_info.push_back(ci);
            }
        } else if (strncmp(l, "AUDIO_L ", 8) == 0) {
            m_audio_out_l = atoi(l + 8);
        } else if (strncmp(l, "AUDIO_R ", 8) == 0) {
            m_audio_out_r = atoi(l + 8);
        } else if (strcmp(l, "READY") == 0) {
            break;
        } else if (strncmp(l, "PROGRAM_COUNT ", 14) == 0) {
            int cnt = atoi(l + 14);
            m_programs.clear();
            m_programs.reserve((size_t)cnt);
        } else if (strncmp(l, "PROGRAM ", 8) == 0) {
            ProgramInfo pi{};
            char name_buf[512] = {};
            int seq_idx = 0;
            if (sscanf(l + 8, "%d %lu %lu %511[^\n]",
                       &seq_idx, &pi.bank, &pi.program, name_buf) >= 3) {
                pi.name = name_buf;
                m_programs.push_back(pi);
            }
        }
    }
    close(pfd[0]);

    if (!ok) {
        if (timed_out) {
            m_last_error = "plugin did not respond within " +
                           std::to_string(kSetupTimeoutMs / 1000) +
                           "s while starting";
        } else if (!err_line.empty()) {
            m_last_error = err_line;
        } else {
            m_last_error = "sandbox exited during startup";
        }
        std::cerr << "[DSSIInstrument] " << m_last_error
                  << " (" << plugin_path << " #" << plugin_idx << ")\n";
        teardown_sandbox();
        return false;
    }
    m_last_error.clear();

    // A plugin without a usable stereo (or mono) output cannot be mixed
    // through the track, so do not pretend the load succeeded.
    if (m_audio_out_l < 0) {
        m_last_error = "plugin has no audio output ports";
        std::cerr << "[DSSIInstrument] " << m_last_error << " (" << plugin_path << ")\n";
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
    std::cerr << "[DSSIInstrument] " << (plugin_name().empty() ? plugin_path : plugin_name())
              << " ready at " << m_sample_rate.load(std::memory_order_relaxed)
              << " Hz, audio out L=" << m_audio_out_l
              << " R=" << m_audio_out_r << "\n";
    return true;
}

void DSSIInstrument::flush_parameters_to_shm()
{
    if (!m_shm_ptr) return;
    shm()->port_count = (uint32_t)std::min(m_ctrl_info.size(), (size_t)DSB_MAX_PORTS);
    for (size_t i = 0; i < m_ctrl_info.size() && i < DSB_MAX_PORTS; ++i)
        shm()->port_values[i] = m_ctrl_info[i].current_value;
}

void DSSIInstrument::wait_for_audio_idle() const
{
    // process() never blocks (it uses sem_trywait), so a couple of audio
    // cycles is normally enough. The cap only guards against a wedged
    // thread; after it we proceed rather than hang the GUI/export thread.
    for (int i = 0; i < 200 && m_processing.load(std::memory_order_acquire) != 0; ++i)
        usleep(1000);
}

bool DSSIInstrument::respawn_for_sample_rate()
{
    if (m_path.empty() || !m_alive.load(std::memory_order_acquire))
        return false;

    const uint32_t hz = m_sample_rate.load(std::memory_order_relaxed);

    // Keep the user-visible state across the re-instantiation: control
    // values are matched by LADSPA port index because the new instance
    // reports them (with rate-dependent ranges) from scratch.
    struct SavedParam { int port; float value; };
    std::vector<SavedParam> saved;
    saved.reserve(m_ctrl_info.size());
    for (const auto& c : m_ctrl_info)
        saved.push_back({c.port_idx, c.current_value});

    // Stop the audio thread from touching the shared block while it is
    // replaced: m_alive=false makes new process() calls bail out, and the
    // counter covers a block that was already in flight.
    m_alive.store(false, std::memory_order_release);
    wait_for_audio_idle();

    set_status(InstrumentStatus::Starting);
    if (!spawn_sandbox(m_path, m_index)) {
        set_status(InstrumentStatus::LoadFailed);
        std::cerr << "[DSSIInstrument] could not re-instantiate '" << m_path
                  << "' at " << hz << " Hz: " << m_last_error << "\n";
        return false;
    }

    for (auto& c : m_ctrl_info) {
        for (const auto& s : saved) {
            if (s.port != c.port_idx) continue;
            // The range may differ at the new rate (e.g. rate-dependent
            // hints), so clamp instead of restoring blindly.
            c.current_value = std::min(std::max(s.value, c.min_v), c.max_v);
            break;
        }
    }
    flush_parameters_to_shm();
    if (m_bank != 0 || m_program != 0) {
        shm()->select_program_bank    = (uint32_t)m_bank;
        shm()->select_program_program = (uint32_t)m_program;
        shm()->select_program_pending = 1;
    }

    set_status(InstrumentStatus::Ok);
    return true;
}

void DSSIInstrument::set_sample_rate(double sr)
{
    if (!(sr > 0.0)) return;
    const uint32_t hz = (uint32_t)(sr + 0.5);
    const uint32_t old = m_sample_rate.exchange(hz, std::memory_order_acq_rel);
    if (old == hz) return;

    // No plugin loaded yet: the rate is picked up by the next spawn because
    // spawn_sandbox() passes m_sample_rate to the sandbox.
    if (m_path.empty()) return;

    if (!m_alive.load(std::memory_order_acquire)) {
        // A crashed/failed sandbox is not restarted implicitly; a later
        // load_plugin() will use the new rate.
        std::cerr << "[DSSIInstrument] sample rate changed from " << old
                  << " to " << hz << " Hz (plugin not running)\n";
        return;
    }

    std::cerr << "[DSSIInstrument] re-instantiating '" << m_path << "' at "
              << hz << " Hz (was " << old << " Hz)\n";
    respawn_for_sample_rate();
}

// ─── Public API ───────────────────────────────────────────────────────────────

bool DSSIInstrument::load_plugin(const std::string& path, int index)
{
    // A reload replaces the shared block and the parameter table; make sure
    // the audio thread is not inside process() while that happens.
    m_alive.store(false, std::memory_order_release);
    wait_for_audio_idle();

    m_path  = path;
    m_index = index;
    m_ctrl_info.clear();
    m_programs.clear();
    m_bank    = 0;
    m_program = 0;
    m_last_error.clear();
    set_status(InstrumentStatus::Starting);
    set_exit_status(0);

    const bool ok = spawn_sandbox(path, index);
    set_status(ok ? InstrumentStatus::Ok : InstrumentStatus::LoadFailed);
    return ok;
}

bool DSSIInstrument::is_alive() const
{
    if (!m_alive.load(std::memory_order_relaxed)) return false;
    const pid_t pid = m_child_pid.load(std::memory_order_relaxed);
    if (pid <= 0) return true;

    // Throttle the syscalls to ~2/sec, and let exactly one thread perform
    // them. clock_gettime is cheap (vDSO); waitpid is not.
    const int64_t now_ms = monotonic_ms();
    int64_t last = m_last_alive_check_ms.load(std::memory_order_relaxed);
    if (now_ms - last < 500) return true;
    if (!m_last_alive_check_ms.compare_exchange_strong(last, now_ms,
                                                       std::memory_order_relaxed))
        return true; // another thread is probing right now

    // Reap instead of signalling. kill(zombie, 0) succeeds, so the previous
    // probe could never observe a crashed sandbox: the child stayed a zombie
    // and the track went permanently silent with no diagnostic anywhere.
    int status = 0;
    pid_t r;
    do { r = waitpid(pid, &status, WNOHANG); } while (r == -1 && errno == EINTR);

    // r == pid: the sandbox exited and we just reaped it.
    // ECHILD:  it was reaped elsewhere — gone either way, so treat as dead
    //          rather than reporting a phantom "alive" forever.
    if (r == pid || (r == -1 && errno == ECHILD)) {
        set_exit_status(r == pid ? status : 0);
        set_status(InstrumentStatus::Crashed);
        m_alive.store(false, std::memory_order_relaxed);
        m_child_pid.store(-1, std::memory_order_relaxed);
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
    // RT-safe: never grow the staging vector in the audio thread.
    // Staging is flushed every block in process(); if a burst overflows
    // the shm slot budget, drop the newest event instead of reallocating.
    if (m_pending_midi.size() >= DSB_MAX_MIDI) return;
    m_pending_midi.push_back({0x90, (uint8_t)chan, note, velocity});
    m_last_note[chan] = note;
}

void DSSIInstrument::note_off(size_t column_index)
{
    const size_t chan = column_index % 16;
    if (m_last_note[chan] == -1) return;
    if (m_pending_midi.size() >= DSB_MAX_MIDI) { m_last_note[chan] = -1; return; }
    m_pending_midi.push_back({0x80, (uint8_t)chan, (uint8_t)m_last_note[chan], 0});
    m_last_note[chan] = -1;
}

void DSSIInstrument::panic()
{
    // Panic can stage up to 16 events; make room instead of reallocating.
    if (m_pending_midi.size() + 16 > DSB_MAX_MIDI)
        m_pending_midi.clear();
    for (size_t chan = 0; chan < 16; ++chan) {
        m_pending_midi.push_back({0xB0, (uint8_t)chan, 123, 0});
        m_last_note[chan] = -1;
    }
}

namespace {
// Marks the calling thread as being inside DSSIInstrument::process() so the
// control thread can wait for it before replacing the shared block.
struct AudioBusyGuard {
    std::atomic<int>& counter;
    explicit AudioBusyGuard(std::atomic<int>& c) : counter(c) {
        counter.fetch_add(1, std::memory_order_acq_rel);
    }
    ~AudioBusyGuard() {
        counter.fetch_sub(1, std::memory_order_acq_rel);
    }
};
} // namespace

void DSSIInstrument::set_block_size(uint32_t frames)
{
    if (frames == 0) return;                       // backend did not tell us
    if (frames > DSB_MAX_FRAMES) frames = DSB_MAX_FRAMES;
    m_block_frames.store(frames, std::memory_order_relaxed);
}

void DSSIInstrument::fifo_reset()
{
    m_fifo_read = 0;
    m_fifo_fill = 0;
}

void DSSIInstrument::fifo_push(const float* l, const float* r, size_t n)
{
    if (n == 0) return;
    if (n > kFifoFrames - m_fifo_fill) n = kFifoFrames - m_fifo_fill;  // never overflow
    size_t w = m_fifo_read + m_fifo_fill;
    for (size_t i = 0; i < n; ++i) {
        const size_t idx = (w + i) & (kFifoFrames - 1);
        m_fifo_l[idx] = l[i];
        m_fifo_r[idx] = r[i];
    }
    m_fifo_fill += n;
}

size_t DSSIInstrument::fifo_pop(float* l, float* r, size_t n, float gain)
{
    const size_t got = std::min(n, m_fifo_fill);
    if (got == 0) return 0;
    if (gain == 1.0f) {
        for (size_t i = 0; i < got; ++i) {
            const size_t idx = (m_fifo_read + i) & (kFifoFrames - 1);
            l[i] = m_fifo_l[idx];
            r[i] = m_fifo_r[idx];
        }
    } else {
        for (size_t i = 0; i < got; ++i) {
            const size_t idx = (m_fifo_read + i) & (kFifoFrames - 1);
            l[i] = m_fifo_l[idx] * gain;
            r[i] = m_fifo_r[idx] * gain;
        }
    }
    m_fifo_read = (m_fifo_read + got) & (kFifoFrames - 1);
    m_fifo_fill -= got;
    return got;
}

void DSSIInstrument::harvest_reply()
{
    if (!m_request_pending || !m_shm_ptr) return;
    DSSIBridgeShm* s = shm();
    if (sem_trywait(&s->sem_done) != 0) return;

    fifo_push(s->audio_l, s->audio_r, m_pending_frames);
    m_pending_frames = 0;
    m_request_pending = false;
    m_wait_samples = 0;
    if (status() == InstrumentStatus::Stalled)
        set_status(InstrumentStatus::Ok);
}

void DSSIInstrument::post_request(uint32_t frames)
{
    DSSIBridgeShm* s = shm();
    s->nframes = frames;
    s->volume  = m_volume;
    flush_parameters_to_shm();

    const uint32_t mc = (uint32_t)std::min(m_pending_midi.size(), (size_t)DSB_MAX_MIDI);
    s->midi_count = mc;
    for (uint32_t i = 0; i < mc; ++i) {
        const auto& me = m_pending_midi[i];
        s->midi_events[i] = {me.type, me.channel, me.note, me.value};
    }
    m_pending_midi.clear();
    s->shutdown = 0;

    m_pending_frames  = frames;
    m_request_pending = true;
    sem_post(&s->sem_req);
}

void DSSIInstrument::process(float* l, float* r, size_t nframes)
{
    AudioBusyGuard busy(m_processing);
    if (nframes > DSB_MAX_FRAMES) nframes = DSB_MAX_FRAMES;
    memset(l, 0, nframes * sizeof(float));
    memset(r, 0, nframes * sizeof(float));

    if (!is_alive()) {
        m_pending_midi.clear();
        // Do not touch the shared block here: teardown_sandbox() may be
        // unmapping it concurrently (the sems are re-initialised on the
        // next spawn anyway).
        m_request_pending = false;
        m_pending_frames  = 0;
        m_wait_samples    = 0;
        fifo_reset();
        return;
    }

    // The engine calls process() with whatever block it needs — the
    // sequencer splits every backend period at tick boundaries, so the size
    // changes from call to call.  The sandbox renders whole blocks
    // asynchronously, so the two are decoupled by a FIFO.  (Copying the
    // previous reply verbatim, as this used to do, truncated it when the new
    // block was shorter and zero-filled it when it was longer: dropped
    // samples and gaps, audible as distortion whenever the transport ran.)
    uint32_t block = m_block_frames.load(std::memory_order_relaxed);
    if (nframes > block) {
        // Host never told us the period (or it changed): use what it asks.
        block = (uint32_t)nframes;
        m_block_frames.store(block, std::memory_order_relaxed);
    }

    harvest_reply();

    // Keep the pipe full: one request in flight, never more than the FIFO
    // can hold, and stop at two blocks so the added latency stays bounded.
    if (!m_request_pending &&
        m_fifo_fill + block <= kFifoFrames &&
        m_fifo_fill < (size_t)block * 2) {
        post_request(block);
    }

    m_wait_samples += nframes;
    fifo_pop(l, r, nframes, m_volume);

    // A sandbox that stops answering silences the track forever, so report
    // it instead of hiding it: 2 s without a reply is a stall, not latency.
    if (m_wait_samples > (uint64_t)m_sample_rate.load(std::memory_order_relaxed) * 2 &&
        status() != InstrumentStatus::Stalled)
        set_status(InstrumentStatus::Stalled);
}

} // namespace disgrace_ns
