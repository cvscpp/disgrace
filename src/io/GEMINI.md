# IO Subsystem

This directory is responsible for managing all input/output operations, with a primary focus on project persistence and file format handling.

## Purpose
The primary responsibilities of this subsystem include:
*   Saving and loading entire `disgrace` projects, including all associated data (samples, patterns, instrument settings).
*   Handling the reading and writing of various audio file formats.
*   Potentially importing data from other music software formats.

## Current Status
*   **Project Archiving:** `project_archive.cpp` and `project_archive.h` indicate that the system for packaging and un-packaging projects into a `.tar.gz` archive (as specified in the main `GEMINI.md`) is being implemented. This is a crucial component for Phase 2: Persistence.
*   **Song Serialization:** `song_serializer.cpp` and `song_serializer.h` are likely responsible for converting the internal song data structures (global settings, track data, patterns, etc.) into a storable format (e.g., JSON) within the project archive.
*   **Audio File Handling:** `audio_file.cpp` and `audio_file.h` provide functionalities for reading and possibly writing common audio file formats (like WAV), which is essential for importing samples and audio recording.

## Next Steps / Future Work
*   **Import Other Formats:** The main `GEMINI.md` mentions importing `renoise xrns`, `xm` songs, and `standard midi files`. Dedicated parsers for these formats would be developed in this subsystem (part of Phase 4).
*   Further integration with the `audio` and `instrument` subsystems to efficiently load and manage samples.
*   Error handling and robust file validation will be important areas of focus.
