/*
 * Disgrace - Digital Audio Workstation
 * Copyright (C) 2025  Miroslav Shaltev
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Shared-memory layout for the DSSI sandbox bridge.
 *
 * One DSSIBridgeShm region is created per plugin instance (by the host
 * process) and mapped into both the host and the sandboxed child.
 *
 * RT protocol (called from the JACK audio thread on the host side):
 *   Host writes:  nframes, volume, port_values, midi_events, midi_count
 *   Host posts:   sem_req
 *   Sandbox runs: plugin, writes audio_l / audio_r
 *   Sandbox posts:sem_done
 *   Host reads:   audio_l / audio_r  (only if sem_timedwait succeeds)
 *
 * If sem_timedwait times out the plugin is considered dead; the host
 * outputs silence for that instrument until the sandbox is restarted.
 */

#pragma once

#include <semaphore.h>
#include <cstdint>

namespace disgrace_ns {

// Upper bounds — must match between host and sandbox (same binary, same ABI).
static constexpr size_t DSB_MAX_FRAMES = 4096;
static constexpr size_t DSB_MAX_PORTS  = 128;
static constexpr size_t DSB_MAX_MIDI   = 64;

// Compact MIDI event passed through shared memory.
// The sandbox converts these to snd_seq_event_t internally.
struct DsbMidiEvent {
    uint8_t type;       // SND_SEQ_EVENT_NOTEON / NOTEOFF / CONTROLLER
    uint8_t channel;
    uint8_t note;       // note number or controller index
    uint8_t value;      // velocity or controller value
};

// Process-shared communication block.  Must be placed at offset 0 of the
// shared memory region so both processes agree on field addresses.
//
// Alignment: audio buffers are placed after all control data so they
// naturally start on a cache-line boundary in practice.  The struct is
// kept POD so it can live directly in shared memory (no vtable, no heap).
struct alignas(64) DSSIBridgeShm {
    // ── synchronisation ────────────────────────────────────────────────
    sem_t    sem_req;           // host  → sandbox: process N frames
    sem_t    sem_done;          // sandbox → host:  audio ready

    // ── control (written by host before sem_req) ────────────────────────
    uint32_t nframes;           // number of frames to process this block
    float    volume;            // post-process gain (0 .. 1+)
    uint32_t port_count;        // number of valid entries in port_values
    float    port_values[DSB_MAX_PORTS]; // indexed by m_control_indices order
    uint32_t midi_count;
    DsbMidiEvent midi_events[DSB_MAX_MIDI];
    uint32_t shutdown;          // set to 1 to ask sandbox to exit cleanly

    // ── audio (written by sandbox before sem_done) ──────────────────────
    float    audio_l[DSB_MAX_FRAMES];
    float    audio_r[DSB_MAX_FRAMES];
};

// Total size of the shared region.
static constexpr size_t DSB_SHM_SIZE = sizeof(DSSIBridgeShm);

} // namespace disgrace_ns
