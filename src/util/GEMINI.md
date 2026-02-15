# Utility Subsystem

This directory contains general-purpose utility functions and data structures used throughout the `disgrace` project.

## Purpose
The primary responsibility of this subsystem is to provide reusable, foundational components that support various other modules without being specific to any single domain (e.e., audio, GUI, sequencer).

## Current Status
*   **Ring Buffer:** `ringbuffer.h` provides a ring buffer implementation, which is a critical data structure for real-time audio applications, often used for efficient and thread-safe data transfer between different processing contexts (e.g., audio thread and worker threads). This aligns with the "Real-time and non-real-time code are strictly separated" principle in the main plan.

## Next Steps / Future Work
As the project evolves, other general-purpose utilities may be added here, such as custom data structures, common algorithms, or helper functions that do not fit into other specialized subsystems.
