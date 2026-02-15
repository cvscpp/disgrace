# Edit Subsystem

This directory likely contains definitions for various editing commands within `disgrace`, particularly for the tracker interface, and potentially the underlying mechanism for undo/redo operations.

## Purpose
The primary responsibilities of this subsystem include:
*   Defining concrete, discrete editing actions that can be performed (e.g., setting notes, clearing blocks).
*   Providing a common interface for these commands, often used in conjunction with an undo/redo stack.

## Current Status
*   **Tracker Editing Commands:** `cmd_clear_block.h`, `cmd_paste_block.h`, and `cmd_set_note.h` clearly define specific commands for manipulating data within the tracker, indicating progress on keyboard-focused tracker editing.
*   **Command Interface:** `edit_command.h` suggests a base class or interface for these commands, which is a standard approach for implementing flexible editing and undo/redo systems.
*   **Undo/Redo Support:** The presence of `undo_stack.h` (either a local one or a reference to `src/core/undo_stack.h`) suggests that these commands are designed to be integrated with an undo/redo mechanism, fulfilling a critical user expectation for editing features.

## Next Steps / Future Work
*   **Audio Editor Commands:** While tracker commands are present, specific command implementations for the audio editor's `cut`, `copy`, `paste`, `crop`, etc., should be considered here or in a related subsystem.
*   Full integration with the GUI (e.g., `tracker_view`) and the `core`'s undo stack (if different from the one here) will be essential to make these commands functional.
