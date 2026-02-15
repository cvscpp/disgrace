# Sequencer Subsystem

This directory forms the core of the tracker engine, responsible for managing musical patterns, notes, timing, and automation data.

## Purpose
The primary responsibilities of this subsystem include:
*   Defining and managing musical patterns, including their structure and content (notes, effects).
*   Controlling the playback timing and tempo of the composition.
*   Handling automation data for various parameters.

## Current Status
*   **Sequencer Core:** `sequencer.cpp` and `sequencer.h` likely implement the main logic for pattern management, playback, and coordination with the audio engine.
*   **Pattern and Note Data:** `pattern.h` and `note.h` define the fundamental data structures for musical patterns, notes, and potentially associated effect commands, aligning with the "Pattern Model" and "Track Model" described in the main plan.
*   **Automation:** `automation.cpp` and `automation.h` provide the framework for storing and processing automation data, allowing dynamic control of parameters over time.
*   **Timing:** `timing.cpp` and `timing.h` handle the precise timing aspects of the sequencer, such as beats per minute (BPM) and lines per beat, ensuring accurate playback synchronization.

## Next Steps / Future Work
*   **Tracker Effects:** The implementation of the "Minimal Tracker Effect Set" (e.g., volume slides, portamento, arpeggio, retrigger) will be integrated here, likely within `note` or related effect processing logic.
*   Integration with the `instrument` subsystem for triggering sounds and the `edit` subsystem for manipulating pattern data will be crucial for a functional tracker.
*   Further development will involve refining the pattern list management and potentially implementing more complex pattern operations.
