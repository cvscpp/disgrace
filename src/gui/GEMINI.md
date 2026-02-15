# GUI Subsystem

This directory contains all the graphical user interface components for `disgrace`, implemented using the FLTK toolkit.

## Purpose
The primary responsibilities of this subsystem include:
*   Providing a lightweight and responsive user interface for all `disgrace` functionalities.
*   Implementing the main application window with detachable views for the tracker, mixer, and audio editor.
*   Presenting data from other subsystems (sequencer, mixer, audio editor) in an interactive and intuitive way.

## Current Status
*   **Main Window:** `main_window.cpp` and `main_window.h` serve as the application's primary window, expected to manage the tabbed interface and detachable sub-windows as outlined in the main `GEMINI.md`.
*   **Tracker View:** `tracker_view.cpp` and `tracker_view.h` provide the visual component for the tracker engine, forming the central interaction point for pattern-based music creation.
*   **Audio Editor View:** `waveform_view.cpp` and `waveform_view.h` implement the visual display for audio waveforms, essential for the sample and audio editing features.
*   **Transport Controls:** `transport_panel.cpp/h` and `transportbar.cpp/h` likely handle the UI elements for playback control (play, stop, tempo, etc.), integrating with the `core/transport` module.
*   **Mixer Model (Partial Mixer UI):** The `model/mixer_model.cpp/h` suggests a data model for the mixer, indicating a structured approach to building the mixer's user interface.

## Next Steps / Future Work
*   **Full Mixer UI:** The complete mixer UI, including channel strips, insert DSP controls, subgroup views, and level meters, needs to be implemented and integrated with the `mixer` and `dsp` subsystems.
*   **Keybinding and Theming:** Implementation of customizable keybinding presets and simple FLTK themes as described in the main plan.
*   **Spectral Analyzer:** Integration of the optional spectral analyzer, possibly feeding data from the `analysis` subsystem.
*   Refinement of UI elements for all views to ensure a smooth, keyboard-driven workflow.
