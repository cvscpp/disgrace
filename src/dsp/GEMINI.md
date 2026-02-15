# DSP Subsystem

This directory contains the implementations for `disgrace`'s built-in digital signal processing (DSP) effects and the mechanism for chaining them.

## Purpose
The primary responsibilities of this subsystem include:
*   Providing a collection of high-quality, real-time safe DSP effects.
*   Implementing a flexible system for ordering and applying these effects in a chain (e.g., per-track insert effects).

## Current Status
*   **DSP Base Interface:** `dsp.h` likely defines the common interface for all DSP modules, ensuring consistency and extensibility.
*   **DSP Chain:** `dsp_chain.cpp` and `dsp_chain.h` implement the core functionality for creating and managing a chain of DSP effects, allowing them to be reordered and enabled/disabled as per the main plan.
*   **Implemented Effects:**
    *   `delay.h`: A simple delay effect, explicitly listed in the `GEMINI.md`'s core DSP list.
    *   `gain.h`: A gain effect, also explicitly listed.

## Next Steps / Future Work
*   **Missing Core DSP Modules:** A significant portion of the planned core DSP modules from `GEMINI.md` are currently not present. These include:
    *   Simple 3-Band EQ
    *   12-Band Graphical EQ
    *   Low-pass Filter
    *   High-pass Filter
    *   Basic Reverb (Schroeder type)
    *   Compressor (simple RMS)
    *   Saturation (optional)
    *   Exciter
    Implementing these effects is a critical task for Phase 2 of the development plan.
*   Integration with the `mixer` subsystem will be essential to expose these effects to the user and apply them to audio tracks.
