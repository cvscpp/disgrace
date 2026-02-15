# Instrument Subsystem

This directory is responsible for defining and managing the various sound-generating instruments within `disgrace`.

## Purpose
The primary responsibilities of this subsystem include:
*   Providing a unified interface for all instrument types.
*   Implementing specific instrument types, such as the sample instrument.
*   Managing a collection of instruments (e.g., an instrument rack).

## Current Status
*   **Unified Instrument Interface:** `instrument.h` likely defines the common interface (`note_on()`, `note_off()`, `process_audio()`, etc.) that all instruments adhere to, as outlined in the main `GEMINI.md`.
*   **Sample Instrument:** `sample_instrument.cpp` and `sample_instrument.h` implement the core sample-based instrument, a foundational component for the DAW. This aligns with Phase 1 development.
*   **Instrument Rack:** `instrument_rack.cpp` and `instrument_rack.h` suggest a container or manager for multiple instruments, allowing for a more complex and organized sound palette.

## Next Steps / Future Work
*   **MIDI Output Instrument:** Implementation for sending MIDI to JACK for external instruments, as described in the main plan, would likely reside here.
*   **Optional Instrument Types:** Development of SF2 instruments or a simple internal synthesizer (Phase 4) would involve adding new implementations that conform to the `instrument.h` interface.
*   Integration with the `sequencer` to trigger notes and the `mixer` for audio routing will be essential.
