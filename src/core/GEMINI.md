# Core Subsystem

This directory houses the central components of `disgrace`, responsible for managing the overall application state, transport control, and inter-subsystem communication.

## Purpose
The primary responsibilities of this subsystem include:
*   Application lifecycle management (startup, shutdown).
*   Global configuration and subsystem initialization.
*   Providing transport control (play, stop, etc.) for the sequencer and audio engine.
*   Managing the main audio output path (Master Bus).
*   Implementing application-wide undo/redo functionality.

## Current Status
*   **Engine Core:** `engine.cpp` and `engine.h` likely encapsulate the main application logic, handling overall control and coordination. `engine_save.cpp` supports saving the engine's state.
*   **Transport Control:** `transport.cpp` and `transport.h` implement the core playback and control mechanisms, essential for interacting with the sequencer and audio. This is a key part of Phase 1 development.
*   **Master Audio Bus:** `MasterBus.cpp` and `MasterBus.h` provide the central mixing point for all audio, routing it to the JACK backend. This is also critical for Phase 1.
*   **Metronome:** `metronome.cpp` and `metronome.h` provide a timing reference for the user and sequencer.
*   **Undo/Redo Stack:** `undo_stack.cpp` and `undo_stack.h` implement a robust undo/redo system, which is crucial for a user-friendly editing experience across the application.
*   **Command Pattern:** `command.h` and `engine_command.h` define an architectural pattern for encapsulating operations, which aids in building a flexible and extensible system, particularly for undo/redo.

## Next Steps / Future Work
The core engine components provide a strong foundation. Future work will involve further integration with other subsystems, ensuring seamless communication and adherence to the strict real-time rules outlined in the main `GEMINI.md`.
