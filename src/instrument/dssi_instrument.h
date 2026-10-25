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

#pragma once
#include "instrument.h"
#include "dssi_bridge_shm.h"
#include <vector>
#include <string>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <unistd.h>

namespace disgrace_ns {

class DSSIInstrument : public Instrument {
public:
    DSSIInstrument(double sample_rate);
    ~DSSIInstrument();

    void note_on(uint8_t note, uint8_t velocity, size_t column_index = 0, size_t offset_samples = 0, uint8_t sample_index = 0) override;
    void note_off(size_t column_index = 0) override;
    void panic() override;
    void set_volume(float vol) override;
    void set_pitch(float freq) override;
    void process(float* l, float* r, size_t nframes) override;

    // DSSI plugins bake the sample rate into instantiate(), so a rate change
    // requires a fresh plugin instance.  Called by the engine whenever the
    // backend/project rate or an export rate changes.
    void set_sample_rate(double sr) override;
    double sample_rate() const { return (double)m_sample_rate.load(std::memory_order_relaxed); }

    // Backend period, used as the sandbox's render block.  A call to
    // process() may be shorter than this (the sequencer splits a period at
    // tick boundaries), so the two are decoupled by an output FIFO.
    void set_block_size(uint32_t frames) override;

    bool load_plugin(const std::string& path, int index = 0);

    // Returns true when a plugin is loaded and the sandbox process is running.
    bool is_alive() const;

    size_t parameter_count() const override { return m_ctrl_info.size(); }
    Parameter get_parameter(size_t index) const override;
    void set_parameter(size_t index, float value) override;

    void load_program(unsigned long bank, unsigned long program);
    struct ProgramInfo {
        unsigned long bank = 0;
        unsigned long program = 0;
        std::string name;
    };
    size_t program_count() const { return m_programs.size(); }
    const ProgramInfo& program_info(size_t index) const { return m_programs[index]; }

    const std::string& path()    const { return m_path; }
    int                index()   const { return m_index; }
    unsigned long      bank()    const { return m_bank; }
    unsigned long      program() const { return m_program; }
    float              volume()  const { return m_volume; }

    // Reason the last load_plugin() failed (empty on success). Written on the
    // GUI thread by load_plugin(), so reading it from the GUI is safe.
    const std::string& last_error() const { return m_last_error; }

protected:
    std::unique_ptr<Voice> create_voice() override { return nullptr; }

private:
    // ── subprocess management ────────────────────────────────────────
    bool spawn_sandbox(const std::string& path, int index);
    void teardown_sandbox();
    static std::string find_sandbox_binary();

    // Re-instantiate the plugin at the current sample rate, keeping the
    // control-port values and program selection.  No-op when nothing is
    // loaded or the sandbox is not running.
    bool respawn_for_sample_rate();
    // Publish m_ctrl_info to the shared block (RT-safe: plain stores).
    void flush_parameters_to_shm();
    // Block (bounded) until no audio thread is inside process().  Used on
    // the control thread before the shared block is replaced.
    void wait_for_audio_idle() const;

    // ── sandbox block handoff ────────────────────────────────────────
    // Collect a finished sandbox block into the FIFO, if one is ready.
    void harvest_reply();
    // Ask the sandbox to render one block of `frames`.
    void post_request(uint32_t frames);
    void fifo_reset();
    void fifo_push(const float* l, const float* r, size_t n);
    // Copies at most n frames; returns how many were available.
    size_t fifo_pop(float* l, float* r, size_t n, float gain);

    // Ring buffer of rendered frames.  Power of two so indices mask; sized
    // for two maximum-size blocks.
    static constexpr size_t kFifoFrames = 8192;
    std::vector<float> m_fifo_l{std::vector<float>(kFifoFrames, 0.0f)};
    std::vector<float> m_fifo_r{std::vector<float>(kFifoFrames, 0.0f)};
    size_t m_fifo_read = 0;
    size_t m_fifo_fill = 0;
    // Frames per sandbox render block (the backend period).
    std::atomic<uint32_t> m_block_frames{512};

    // Atomic: written by spawn/teardown on the GUI thread and cleared by the
    // RT thread's liveness probe when it reaps a dead child, so mutable.
    mutable std::atomic<pid_t> m_child_pid{-1};
    void*   m_shm_ptr    = nullptr;
    std::string m_shm_name;
    std::string m_last_error;
    DSSIBridgeShm* shm() { return static_cast<DSSIBridgeShm*>(m_shm_ptr); }

    // ── per-parameter info (filled during load_plugin from child stdout) ─
    struct CtrlInfo {
        int   port_idx;   // LADSPA port index (informational)
        float min_v, max_v, def_v;
        std::string name;
        float current_value;
    };
    std::vector<CtrlInfo> m_ctrl_info;
    std::vector<ProgramInfo> m_programs;

    // ── audio routing reported by child ─────────────────────────────
    int  m_audio_out_l = -1;
    int  m_audio_out_r = -1;
    bool m_has_run_synth = false;

    // ── runtime state ────────────────────────────────────────────────
    // Written by the control thread (engine/export/GUI), read by the audio
    // thread for the stall timeout, hence atomic.
    std::atomic<uint32_t> m_sample_rate{44100};
    std::string   m_path;
    int           m_index   = 0;
    unsigned long m_bank    = 0;
    unsigned long m_program = 0;
    float         m_volume  = 0.25f;

    // Number of threads currently inside process().  The control thread
    // waits for this to drop to zero before tearing down the shared block.
    std::atomic<int> m_processing{0};

    // MIDI event staging (GUI/audio thread writes, flushed in process())
    struct StagedMidi { uint8_t type, channel, note, value; };
    std::vector<StagedMidi> m_pending_midi;
    int m_last_note[16]{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
    // Audio thread only. The sandbox runs one block behind so process()
    // never waits on the plugin.
    bool m_request_pending = false;
    uint32_t m_pending_frames = 0;
    // Frames spent waiting for a reply to the outstanding request; used to
    // tell a slow plugin from a hung one.
    uint64_t m_wait_samples = 0;

    // ── crash state ──────────────────────────────────────────────────
    // kill() is a syscall: throttle the liveness probe so the RT thread
    // only pays it ~2x/sec instead of once per audio block.
    mutable std::atomic<bool> m_alive{false};
    mutable std::atomic<int64_t> m_last_alive_check_ms{0};
};

} // namespace disgrace_ns
