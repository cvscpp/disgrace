# MIDI Subsystem

This directory manages MIDI input and potentially output functionalities for `disgrace`.

## Purpose
The primary responsibilities of this subsystem include:
*   Receiving and processing MIDI input events from devices.
*   Providing a robust mechanism for handling MIDI data, often in a real-time safe manner.

## Current Status
*   **MIDI Input:** `midi_input.cpp` and `midi_input.h` suggest that MIDI input handling is implemented or under development.
*   **MIDI Event Queue:** `midi_queue.h` indicates the use of a queue to manage MIDI events, which is crucial for decoupling MIDI input from real-time audio processing and ensuring stability.

## Next Steps / Future Work
*   **MIDI Output Instrument:** While input is present, explicit components for the "MIDI Output Instrument" mentioned in the main `GEMINI.md` are not immediately obvious. This might be handled within existing files or will be a future addition.
*   Integration with the `instrument` and `sequencer` subsystems will be key for utilizing MIDI data effectively.
