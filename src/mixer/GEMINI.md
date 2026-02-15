# Mixer Subsystem

This directory contains the foundational components for the mixer section of `disgrace`, primarily focusing on individual audio tracks.

## Purpose
The primary responsibilities of this subsystem include:
*   Defining the structure and properties of individual mixer tracks.
*   Managing per-track parameters such as volume, pan, and mute/solo states.
*   Providing the mechanism for integrating DSP insert chains on each track.

## Current Status
*   **Mixer Track:** `track.cpp` and `track.h` represent the core implementation of an individual mixer track. These files are expected to define the basic channel structure (volume, pan, mute, solo) and serve as the integration point for DSP insert effects, aligning with Phase 1 development.

## Next Steps / Future Work
*   **DSP Insert Chain Integration:** A critical next step is to integrate the DSP effect chain (defined in `src/dsp`) into each `track` object.
*   **Routing:** Implementing the routing logic from tracks to optional subgroups and then to the `core/MasterBus`.
*   **Mixer UI Integration:** The `mixer` subsystem needs to be fully exposed and controlled via the GUI, leveraging components from `src/gui/model/mixer_model.h/cpp` for its visual representation.
*   Implementing additional mixer features like level meters, and supporting the optional spectral analyzer.
