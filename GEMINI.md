# Disgrace - A Minimalist Tracker DAW

This document outlines the development plan for `disgrace`, a lightweight, tracker-style digital audio workstation (DAW). It is based on the initial feature ideas in `disgrace.init` and the detailed technical specification in `disgrace.plan`.

## 1. Project Vision

The core vision is to create a stable, focused, and musically useful tool, prioritizing simplicity and performance over a massive feature set. This is not intended to be a clone of existing DAWs or a host for a vast plugin ecosystem.

**Key Principles:**

*   **Stability over Extensibility:** Favor built-in features over external plugins.
*   **Minimalism:** A clean, keyboard-driven workflow with a focused set of DSP effects.
*   **Real-Time Safety:** Strict separation between real-time audio processing and other tasks.
*   **Clear Architecture:** Well-defined, decoupled subsystems.

## 2. Core Architecture

The application will be built on a clear, multi-threaded architecture:

*   **Audio Backend:** [JACK](https://jackaudio.org/) for audio and MIDI I/O.
*   **GUI:** [FLTK](https://www.fltk.org/) for a lightweight, cross-platform interface. The UI will be detachable (tracker, mixer, editor).
*   **Core Engine:** Manages application state, transport, and communication between threads.
*   **Threading Model:**
    *   **Audio Thread:** Real-time, lock-free audio processing.
    *   **GUI Thread:** Manages the user interface.
    *   **Worker Thread(s):** For background tasks like file I/O and sample processing.

## 3. Key Subsystems & Features

### Sequencer / Tracker
*   **Pattern-based:** 512 rows max per pattern.
*   **Tracks:** Each track hosts a single instrument and can have up to 16 sub-tracks (note columns).
*   **Effects:** A minimal, curated set of musical effects (e.g., volume/pan slides, portamento, arpeggio, retrigger).

### Instrument Subsystem
A unified interface will support several instrument types:
1.  **Sample Instrument:** A RAM-based sampler with ADSR, pitch, and looping.
2.  **MIDI Output:** For controlling external hardware or software.
3.  **(Future) SF2 Instrument:** Support for SoundFont2 files.
4.  **(Future) Simple Internal Synth:** A basic built-in synthesizer.

### Mixer & DSP
*   **Internal DSP:** A small, high-quality set of built-in effects will be favored over LADSPA/LV2/DSSI plugins.
*   **Core Effects:** Gain, 3-Band EQ, 12-Band Graphical EQ, Filters (LP/HP), Delay, Reverb, Compressor, Exciter.
*   **Structure:** Each track has a simple channel strip with an insert chain for DSP modules.

### Audio Editor
*   Offline (non-real-time) sample editing.
*   **Features:** Record from JACK input, cut/copy/paste, normalize, crop, time-stretch (`libsndtouch`), and resample (`libsamplerate`).

### File Format
*   **Container:** A `.tar.gz` archive.
*   **Manifest:** A `song.json` file describing the project structure, settings, patterns, and instrument/mixer state.
*   **Assets:** Samples and other assets will be stored in subdirectories within the archive.
*   **Import:** Support for importing `.wav`, MIDI, and potentially `.xm` files is planned.

## 4. Development Plan

Development will proceed in logical phases:

*   **Phase 1: Core Playback**
    *   Implement the JACK audio engine.
    *   Create the basic sample instrument.
    *   Build the core tracker for pattern playback.
    *   Basic mixer with volume and pan controls.

*   **Phase 2: DSP and Persistence**
    *   Integrate the built-in DSP modules (EQ, Delay, etc.).
    *   Implement project saving and loading (`.tar.gz` format).
    *   Enable audio recording into the sample editor.

*   **Phase 3: Editing and Control**
    *   Build out the sample editor features (cut, normalize, etc.).
    *   Implement the instrument macro system for automation from the tracker.
    *   Add global transport effects (tempo changes, pattern jumps).

*   **Phase 4: Expansion**
    *   (Optional) Add an SF2-based instrument or a simple internal synth.
    *   (Optional) Implement importers for XM or Standard MIDI Files.

This phased approach ensures that a functional core is available early and can be built upon incrementally.
