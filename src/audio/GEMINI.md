# Audio Subsystem

This directory contains the core audio processing components for `disgrace`.

## Purpose
The primary responsibilities of this subsystem include:
*   Providing the low-level audio and MIDI backend (JACK).
*   Managing audio samples and their playback.
*   Implementing audio processing utilities like time-stretching and resampling.

## Current Status
*   **JACK Backend:** `jack_backend.cpp` and `jack_backend.h` indicate a JACK audio and MIDI integration is in place.
*   **Sample Management:** `sample_buffer.h/cpp`, `sample_data.h`, and `sample_voice.h/cpp` provide the foundational elements for handling audio samples and their playback. `adsr.h` is present for envelope generation.
*   **Audio Utilities:** `timestretch.h/cpp` (likely using `libsndtouch` as per the main plan) and `resampler.h/cpp` (likely using `libsamplerate`) are implemented, supporting features required by the audio editor.

## Next Steps / Future Work
The current implementation provides a strong foundation. Further development will likely focus on integrating these components into the higher-level instrument and mixer subsystems, as well as refining performance and stability.
