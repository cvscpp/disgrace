# Disgrace - Copilot Instructions

## Project Overview

**Disgrace** is a minimalist, tracker-style digital audio workstation (DAW) focused on simplicity, stability, and real-time safety. It is **not** a clone of existing DAWs and does **not** aim to be extensible via plugins. Instead, it provides a stable, focused set of built-in features with a clean, keyboard-driven interface.

### Key Philosophy
- **Stability over Extensibility:** Built-in features are preferred over external plugins.
- **Minimalism:** A focused set of DSP effects and a clean, keyboard-driven workflow.
- **Real-Time Safety:** Strict separation between real-time audio and other tasks.
- **Clear Architecture:** Well-defined, decoupled subsystems.

## Technology Stack

| Component | Technology |
|-----------|-----------|
| **Audio Backend** | JACK (Audio & MIDI) |
| **GUI Framework** | wxWidgets |
| **Configuration** | Autotools (autoconf/automake) |
| **Language** | C++17 |
| **Dependencies** | nlohmann/json, libsndfile, libsamplerate, FFTW3, libarchive, FluidSynth, soundtouch, wxWidgets |

## Project Structure

```
disgrace/
├── src/
│   ├── audio/           # JACK backend, sample voices, audio processing
│   ├── sequencer/       # Tracker engine, patterns, timing
│   ├── instrument/      # Sample, MIDI, SF2, LV2/DSSI instruments
│   ├── mixer/           # Track/bus structure
│   ├── dsp/             # Built-in DSP modules (EQ, delay, reverb, etc.)
│   ├── gui/             # wxWidgets GUI (tracker, mixer, editor panels)
│   ├── core/            # Engine, transport, config, key bindings
│   ├── io/              # File I/O, project format, XRNS import
│   ├── edit/            # Edit commands for undo/redo
│   ├── analysis/        # FFT analysis
│   └── main.cpp         # Application entry point
├── configure.ac         # Autoconf configuration
├── disgrace.plan        # Detailed technical specification
├── disgrace.init        # Initial feature ideas
└── examples/            # Example projects
```

## Core Architecture

### Threading Model

**Real-Time Thread:**
- JACK audio callback (no memory allocation, no locks, no I/O)
- Processes audio buffers, applies DSP, runs sample voices

**GUI Thread:**
- wxWidgets main loop
- Handles user input and rendering

**Worker Thread(s):**
- File I/O (project save/load)
- Sample processing (time-stretch, resample)
- Heavy DSP operations

**Inter-thread Communication:**
- Lock-free queues for commands
- Double-buffered state blocks for safe state updates

### Key Subsystems

#### 1. **Audio Engine** (`src/audio/`)
- JACK backend for audio/MIDI I/O
- Sample buffers and voice playback
- Resampler and time-stretch support
- No dynamic allocation in real-time path

#### 2. **Sequencer** (`src/sequencer/`)
- Pattern-based tracker (max 512 rows per pattern)
- 16 max subtracks per track
- Row-based effect commands
- Transport effects (tempo, pattern break/jump)

#### 3. **Instruments** (`src/instrument/`)
- **Sample Instrument:** RAM-based stereo samples with ADSR and looping
- **MIDI Instrument:** Sends MIDI to JACK outputs
- **SF2 Instrument:** SoundFont2 support (FluidSynth)
- **LV2/DSSI Plugins:** Limited plugin support

#### 4. **Mixer** (`src/mixer/`)
- Track structure with volume, pan, mute, solo
- Insert DSP chains (max 4 effects per track)
- Subgroup/bus routing
- Built-in DSP modules (no external plugins for core effects)

#### 5. **Built-in DSP** (`src/dsp/`)
Curated set of high-quality effects:
- **Gain, EQ** (3-band, graphical)
- **Filters** (LP/HP)
- **Delay, Reverb**
- **Compressor, Limiter**
- **Saturation, Distortion**
- **Specials:** Exciter, Chorus, Flanger, Phaser, Ring Modulator, Stereo Expander

#### 6. **GUI** (`src/gui/`)
- **wxWidgets-based** with detachable panels
- **Tracker Panel:** Keyboard-driven pattern editor
- **Mixer Panel:** Channel strips with inline DSP controls
- **Audio Editor:** Record, cut/copy/paste, normalize, time-stretch, resample
- **Project Panel:** File operations and settings
- **Spectral View:** FFT-based frequency analysis per channel or master
- **Keybinding Presets:** FastTracker 2, Impulse Tracker, Renoise (fully customizable)

#### 7. **File Format** (`src/io/`)
- **Container:** tar.gz archive
- **Manifest:** song.json (text-based for debugging)
- **Structure:** Patterns, instruments, mixer settings, samples
- **Importers:** XRNS (Renoise), MIDI, WAV

## Development Phases

### Phase 1: Core Playback ✓ (In Progress)
- [x] JACK audio engine
- [x] Sample instrument with basic playback
- [x] Pattern playback
- [ ] Basic mixer (volume/pan)

### Phase 2: DSP & Persistence
- [ ] Built-in DSP modules
- [ ] Project save/load
- [ ] Audio recording

### Phase 3: Editing & Control
- [ ] Sample editor features
- [ ] Instrument macro system
- [ ] Global transport effects

### Phase 4: Expansion (Optional)
- [ ] SF2 instrument
- [ ] Simple internal synth
- [ ] XM import

## Code Style & Guidelines

### C++ Standards
- **C++17** required
- Prefer modern C++ idioms (smart pointers, RAII, const-correctness)
- Use `std::` for standard library features

### Real-Time Safety
Code in the audio callback (`src/audio/jack_backend.cpp`) must follow strict rules:
```cpp
// ❌ FORBIDDEN in real-time path:
new, delete, malloc, free          // No allocation
std::lock, std::mutex              // No locks
FILE I/O, socket operations        // No I/O
GUI updates, logging               // No system calls

// ✅ ALLOWED in real-time path:
Fixed-size arrays, pre-allocated buffers
Lock-free queues (for commands from other threads)
DSP calculations, sample playback
```

### Organization
- **Headers (.h):** Interface, inline functions, template definitions
- **Implementation (.cpp):** Implementation details
- **Separation:** Real-time code strictly separated from non-real-time code
- **Namespacing:** Code in `disgrace_ns` namespace

### Comments
- Only comment code that needs clarification
- Avoid obvious comments
- Document threading constraints (real-time vs. safe-to-call-from-GUI)

## Building & Testing

### Build Commands
```bash
# Configure (first time or after configure.ac changes)
./autogen.sh
./configure [--enable-debug] [--enable-sndtouch]

# Build
make

# Clean
make clean

# Install (optional)
make install
```

### Running
```bash
# Launch the application
./src/disgrace

# With JACK running in the background:
jackd -d alsa &
./src/disgrace
```

### Dependencies Installation

**Ubuntu/Debian:**
```bash
sudo apt-get install libwxgtk3.0-gtk3-dev libjack-jackd2-dev \
  liburiparser-dev libfftw3-dev libsndfile1-dev \
  libsamplerate0-dev libarchive-dev libfluidsynth-dev \
  nlohmann-json3-dev libsndtouch-dev
```

## Key Concepts

### Pattern & Sequencing
- **Pattern:** Fixed-size grid (max 512 rows, configurable lines per beat)
- **Track:** Single instrument + up to 16 subtracks (note columns)
- **Row:** Single line in the pattern, contains note + optional effect command
- **Effect Command:** Tracker-style effect (volume slide, portamento, arpeggio, etc.)

### Tracking Effects
Minimal, musically meaningful set:
- **Transport:** Set tempo, pattern break/jump, pattern delay
- **Amplitude:** Volume set/slide, fade
- **Pitch:** Portamento, arpeggio, pitch offset
- **Panning:** Pan set/slide
- **Timing:** Retrigger, note delay
- **Instrument:** Macro control (4-8 parameters per instrument)

### Real-Time Processing
The JACK callback (`jack_backend.cpp`) runs at ~1ms intervals on dedicated real-time thread:
1. Query input buffers (MIDI, audio)
2. Advance sequencer (sample accuracy)
3. Generate voices (instrument playback)
4. Apply DSP chain per track
5. Mix to master output
6. Write output buffers

## Common Development Tasks

### Adding a New DSP Effect
1. Create header in `src/dsp/myeffect.h`
2. Inherit from base DSP interface (see `src/dsp/dsp.h`)
3. Implement `process()` (non-real-time-safe can use locks)
4. Register in `DspChain::create_module()` (`src/dsp/dsp_chain.cpp`)
5. Add GUI control in mixer panel (`src/gui/wx_mixer_panel.cpp`)

### Adding Tracker Effects
1. Define effect code in `src/sequencer/sequencer.h`
2. Implement handler in `src/sequencer/sequencer.cpp`
3. Update tracker GUI to support input
4. Document in keybinding presets

### Saving/Loading Projects
- Entry point: `src/io/song_serializer.cpp`
- Archive handling: `src/io/project_archive.cpp`
- Serialization uses nlohmann/json

## Important Files

| File | Purpose |
|------|---------|
| `src/main.cpp` | Application entry point |
| `src/core/engine.h/.cpp` | Main application engine |
| `src/audio/jack_backend.h/.cpp` | JACK audio callback |
| `src/sequencer/sequencer.h/.cpp` | Tracker playback engine |
| `src/mixer/track.h/.cpp` | Track structure |
| `src/instrument/sample_instrument.h/.cpp` | Sample playback |
| `src/gui/wx_main_window.h/.cpp` | Main GUI window |
| `src/dsp/dsp_chain.h/.cpp` | DSP module chain |
| `src/io/song_serializer.h/.cpp` | Save/load logic |

## Documentation References

- **`disgrace.plan`** - Detailed technical specification (62 sections)
- **`disgrace.init`** - Initial feature ideas and rough notes
- **`GEMINI.md`** files - Per-subsystem documentation
- **`LICENSE`** - GPL3 (honor the license in contributions)

## Tips for Contributors

1. **Before making changes:** Review the relevant GEMINI.md file for subsystem overview
2. **Real-time safety:** If touching audio callback, audit for allocations/locks
3. **State changes:** Use engine commands for thread-safe state updates
4. **GUI updates:** Always post updates to GUI thread from worker/audio threads
5. **Testing:** Verify with JACK running; test with various sample rates/buffer sizes
6. **Git workflow:** Include meaningful commit messages with technical details

## Common Pitfalls

❌ **Allocating memory in audio callback**
```cpp
// BAD
std::vector<float> buffer(size);  // Allocation in RT path!
```

❌ **Using mutexes in real-time code**
```cpp
// BAD
std::lock_guard<std::mutex> lock(m_mutex);  // Blocking in RT!
```

✅ **Pre-allocate and lock-free**
```cpp
// GOOD
std::array<float, MAX_SIZE> buffer;  // Pre-allocated
queue.push_command(cmd);              // Lock-free operation
```

---

**Last Updated:** 2026-04-02  
**Current Version:** 0.1.0 (Early Development)
