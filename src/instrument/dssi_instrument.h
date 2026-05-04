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

    bool load_plugin(const std::string& path, int index = 0);

    // Returns true when a plugin is loaded and the sandbox process is running.
    bool is_alive() const;

    size_t parameter_count() const override { return m_ctrl_info.size(); }
    Parameter get_parameter(size_t index) const override;
    void set_parameter(size_t index, float value) override;

    void load_program(unsigned long bank, unsigned long program);

    const std::string& path()    const { return m_path; }
    int                index()   const { return m_index; }
    unsigned long      bank()    const { return m_bank; }
    unsigned long      program() const { return m_program; }
    float              volume()  const { return m_volume; }

protected:
    std::unique_ptr<Voice> create_voice() override { return nullptr; }

private:
    // ── subprocess management ────────────────────────────────────────
    bool spawn_sandbox(const std::string& path, int index);
    void teardown_sandbox();
    static std::string find_sandbox_binary();

    pid_t   m_child_pid  = -1;
    int     m_stdout_rfd = -1;   // read end of child's stdout pipe
    void*   m_shm_ptr    = nullptr;
    std::string m_shm_name;
    DSSIBridgeShm* shm() { return static_cast<DSSIBridgeShm*>(m_shm_ptr); }

    // ── per-parameter info (filled during load_plugin from child stdout) ─
    struct CtrlInfo {
        int   port_idx;   // LADSPA port index (informational)
        float min_v, max_v, def_v;
        std::string name;
        float current_value;
    };
    std::vector<CtrlInfo> m_ctrl_info;

    // ── audio routing reported by child ─────────────────────────────
    int  m_audio_out_l = -1;
    int  m_audio_out_r = -1;
    bool m_has_run_synth = false;

    // ── runtime state ────────────────────────────────────────────────
    double        m_sample_rate;
    std::string   m_path;
    int           m_index   = 0;
    unsigned long m_bank    = 0;
    unsigned long m_program = 0;
    float         m_volume  = 0.25f;

    // MIDI event staging (GUI/audio thread writes, flushed in process())
    struct StagedMidi { uint8_t type, channel, note, value; };
    std::vector<StagedMidi> m_pending_midi;
    int m_last_note[16]{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

    // ── crash state ──────────────────────────────────────────────────
    mutable std::atomic<bool> m_alive{false};
};

} // namespace disgrace_ns
