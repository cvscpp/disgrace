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
#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>
#include <array>
#include <atomic>
#include "../audio/voice.h"
#include "../sequencer/note.h"

namespace disgrace_ns
{

constexpr size_t MAX_VOICES = 32;

enum class InstrumentType {
    None,
    Sampler,
    SoundFont,
    SFZ,
    Plugin,
    Midi,
    Voice,
    XRNI
};

// Health of an instrument's audio backend. Instruments that run in-process
// are always Ok; only out-of-process backends (the DSSI sandbox) ever report
// anything else. Reading the status is RT-safe and allocation-free, so the
// GUI can poll it from its timer and the audio thread can update it.
enum class InstrumentStatus : uint8_t {
    Ok,          // working
    Starting,    // backend is being prepared (load in progress)
    LoadFailed,  // backend could not be started
    Crashed,     // backend process exited on its own
    Stalled      // backend is alive but has stopped producing audio
};

class Instrument
{
public:
    virtual ~Instrument() = default;

    virtual void note_on(uint8_t note, uint8_t velocity, size_t column_index = 0, size_t offset_samples = 0, uint8_t sample_index = 0) = 0;
    virtual void note_off(size_t column_index = 0) = 0;
    virtual void panic() = 0;

    virtual void set_sample_rate(double sr) {}

    // Hint from the engine: number of frames the audio backend asks for per
    // callback (0 = unknown). Instruments that hand audio to a helper
    // process render in whole blocks of this size.
    virtual void set_block_size(uint32_t) {}

    virtual void set_volume(float vol) = 0;

    virtual void set_pitch(float freq) = 0;
    virtual void process(float* l,
                         float* r,
                         size_t nframes) = 0;

    virtual void set_audio_input(int, int) {}
    virtual void get_audio_input(int& channel_l, int& channel_r) const
    {
        channel_l = -1;
        channel_r = -1;
    }

    void set_name(const std::string& name) { m_name = name.substr(0, 64); }
    const std::string& name() const { return m_name; }

    void set_plugin_name(const std::string& name) { m_plugin_name = name; }
    const std::string& plugin_name() const { return m_plugin_name; }

    void set_type(InstrumentType type) { m_type = type; }
    InstrumentType type() const { return m_type; }

    struct Parameter {
        int index;
        std::string name;
        float min = 0.0f;
        float max = 1.0f;
        float value = 0.0f;
    };

    virtual size_t parameter_count() const { return 0; }
    virtual Parameter get_parameter(size_t) const { return {}; }
    virtual void set_parameter(size_t, float) {}

    // ── backend health ───────────────────────────────────────────────
    InstrumentStatus status() const {
        return m_status.load(std::memory_order_relaxed);
    }
    // Raw waitpid() status of a backend that exited by itself, 0 if none.
    // The UI can turn this into "exit code N" / "killed by signal S".
    int exit_status() const {
        return m_exit_status.load(std::memory_order_relaxed);
    }

protected:
    virtual ::std::unique_ptr<disgrace_ns::Voice> create_voice() = 0;

    disgrace_ns::Voice* allocate_voice(size_t column_index);

    // Cheap enough for the audio thread: a relaxed store, no allocation, no
    // locking. const so backend liveness probes stay const-correct.
    void set_status(InstrumentStatus s) const {
        m_status.store(s, std::memory_order_relaxed);
    }
    void set_exit_status(int s) const {
        m_exit_status.store(s, std::memory_order_relaxed);
    }

    std::string m_name = "New Instrument";
    std::string m_plugin_name = "";
    InstrumentType m_type = InstrumentType::None;
protected:
    ::std::array<::std::unique_ptr<disgrace_ns::Voice>, MAX_VOICES> m_voices;

private:
    // Written by whichever thread owns the backend (RT thread for a crash,
    // GUI thread for load results), read from the GUI timer.
    mutable ::std::atomic<InstrumentStatus> m_status{InstrumentStatus::Ok};
    mutable ::std::atomic<int> m_exit_status{0};
};

class NoneInstrument : public Instrument {
public:
    void note_on(uint8_t, uint8_t, size_t, size_t, uint8_t) override {}
    void note_off(size_t) override {}
    void panic() override {}
    void set_volume(float) override {}
    void set_pitch(float) override {}
    void process(float*, float*, size_t) override {}
protected:
    std::unique_ptr<Voice> create_voice() override { return nullptr; }
};

} // namespace disgrace_ns
