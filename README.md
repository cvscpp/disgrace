# Disgrace

![Disgrace logo](./imgs/disgrace.png)

**Disgrace** is a minimalist tracker-style digital audio workstation focused on fast keyboard-driven composition, clear structure, and real-time-safe audio design.

It is built around:

- **JACK** for audio and MIDI I/O
- **wxWidgets** for the desktop UI
- A **pattern-based tracker workflow**
- A compact, built-in **mixer / DSP** approach instead of a large plugin-first design

## Project goals

Disgrace is intended to be a small, musically useful DAW rather than a sprawling host platform. The design prioritizes:

- stability over extensibility
- a tracker-centric workflow
- human-scale complexity
- real-time-safe architecture
- built-in tools for composition, editing, and mixing

The planned shape of the application includes:

- pattern sequencing with up to 512 rows per pattern
- tracks with multiple note columns / subtracks
- sample, MIDI, and soundfont-oriented instruments
- a mixer with inserts and internal DSP
- sample recording and offline audio editing
- import/export around a self-contained project format

## Current architecture

The codebase is organized around a few main subsystems:

- **Core engine** for transport, state, configuration, and coordination
- **Audio backend** for JACK-based playback, recording, and MIDI
- **Sequencer / tracker** for pattern playback and editing
- **Instrument subsystem** for sample and external instrument workflows
- **Mixer / DSP** for channel processing and routing
- **wxWidgets GUI** with tabbed views such as project, tracker, tracks, notation, instruments, mixer, settings, and help

## Build

This project uses the GNU autotools build flow.

```bash
./configure
make
```

The dependency set is centered on:

- `wxWidgets`
- `JACK`
- `libsndfile`
- `libsamplerate`
- `soundtouch`
- `fftw3`
- `libarchive`
- `FluidSynth`
- `ALSA`
- `libxml2`

Some optional or platform-specific integrations are also referenced in the tree.

## Running

After building, launch the application with:

```bash
./src/disgrace
```

At startup, Disgrace now shows an intro window with the logo, current version, license summary, warranty disclaimer, and a **Continue** button before the main application opens.

Because Disgrace is designed around JACK, running it in a JACK-enabled audio environment is the expected setup.

## Design and authorship note

Disgrace is **human-designed** in its concept, product direction, architecture, and feature decisions.

At the same time, this repository is intentionally presented as **fully LLM-coded**, including the implementation work and the graphics art such as the project logo. In short: the software direction is human-authored, while the produced code and visual assets are LLM-generated.

## License

This project is licensed under the **GNU General Public License v3**. See [LICENSE](./LICENSE).
