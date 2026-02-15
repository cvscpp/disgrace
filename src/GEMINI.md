# src Directory Overview

This directory contains the source code for the `disgrace` digital audio workstation, organized into various subsystems as detailed in the top-level `GEMINI.md` file. Each subdirectory within `src` contains a more specific `GEMINI.md` file that elaborates on its purpose, current status, and future development based on the project plan.

## Overall Progress Summary

Based on the file structure and the main `GEMINI.md` plan:

*   **Phase 1 (Core Playback)** appears to be well underway.
    *   The **JACK audio engine** (`src/audio`) and **MIDI input** (`src/midi`) foundations are in place.
    *   The **sample instrument** (`src/instrument`) is implemented.
    *   The **core tracker** (`src/sequencer`) with its pattern and note handling is present.
    *   The **basic mixer** (`src/mixer`) with track definitions is established, along with the **Master Bus** (`src/core`).
    *   Key **GUI components** (`src/gui`) for the main window, tracker view, waveform view, and transport controls are initiated.
    *   The **Core Engine** (`src/core`) managing overall application flow and transport is robust.

*   **Phase 2 (DSP and Persistence)** has started:
    *   Basic **DSP modules** (`src/dsp`) like delay and gain are present, but many core effects are still missing. This is a significant area for future development.
    *   The **project saving and loading** mechanism (`src/io`) using `.tar.gz` archives and `song_serializer` is being built.
    *   **Audio recording** (`src/io/audio_file`) is likely supported through file handling.

*   **Phase 3 (Editing and Control)** has preliminary components:
    *   **Sample editor** GUI (`src/gui/waveform_view`) and underlying audio processing utilities (`src/audio/timestretch`, `src/audio/resampler`) are present.
    *   **Tracker editing commands** (`src/edit`) are defined, supported by a general **undo/redo stack** (`src/core`).

*   **Phase 4 (Expansion)** features, such as additional instrument types (SF2, internal synth) and more complex file importers (XRNS, XM, standard MIDI), are not yet visible in the codebase, which aligns with their "optional/future" status in the plan.

## Key Areas for Continued Development

1.  **DSP Module Implementation:** Completing the set of built-in DSP effects (EQs, Filters, Reverb, Compressor, Exciter, Saturation) in `src/dsp` is crucial for the mixer's functionality.
2.  **Mixer UI and DSP Integration:** Fully integrating the DSP chains into `src/mixer/track` and exposing comprehensive controls through the `src/gui` will be a major step.
3.  **Advanced Importers:** Implementing parsers for external song formats (XRNS, XM, MIDI) in `src/io`.
4.  **GUI Refinements:** Enhancing the GUI with detailed mixer controls, keybinding customizations, and potentially a spectral analyzer (`src/analysis` integration).

---
*For a comprehensive understanding of the project vision and detailed development plan, please refer to the top-level `GEMINI.md` file in the project root.*
