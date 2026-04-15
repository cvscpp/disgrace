#include "wx_help_panel.h"
#include "../core/engine.h"

#include <wx/sizer.h>

namespace disgrace_ns {

HelpPanel::HelpPanel(wxWindow* parent, Engine& engine)
    : wxPanel(parent, wxID_ANY), m_engine(engine), m_html_win(nullptr)
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    m_html_win = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO);

    main_sizer->Add(m_html_win, 1, wxEXPAND | wxALL, 0);
    SetSizer(main_sizer);

    load_documentation();
}

void HelpPanel::load_documentation() {
    if (!m_html_win) return;

    wxString html = R"(
<html>
<body bgcolor="#1e1e1e" text="#d4d4d4" link="#569cd6" vlink="#4ec9b0">
<h1>Disgrace &mdash; Digital Audio Workstation</h1>
<p>A minimalist, pattern-based tracker DAW with selectable JACK or OSS audio backends. All tabs are accessible from the top of the main window.</p>

<h2>Interface</h2>
<table width="100%" border="0" cellpadding="4">
<tr><td valign="top"><b>Project</b></td><td>Manage tracks and buses. Set instrument, notation type, output bus, velocity humanization and timing humanization per track.</td></tr>
<tr><td valign="top"><b>Tracker</b></td><td>Pattern editor. Enter notes, volumes, effects. Supports multiple note sub-columns per track.</td></tr>
<tr><td valign="top"><b>Tracks</b></td><td>Arrange patterns on a song timeline.</td></tr>
<tr><td valign="top"><b>Notation</b></td><td>Piano roll view of the current pattern.</td></tr>
<tr><td valign="top"><b>Instrument</b></td><td>Load and manage instruments and samples.</td></tr>
<tr><td valign="top"><b>Mixer</b></td><td>Per-track and bus gain, panning, DSP effects chain and spectrum analyzer.</td></tr>
<tr><td valign="top"><b>Settings</b></td><td>Audio backend, MIDI, theme and GUI settings.</td></tr>
<tr><td valign="top"><b>Help</b></td><td>This documentation.</td></tr>
</table>

<h2>Keyboard Shortcuts</h2>

<h3>Transport</h3>
<table width="100%" border="0" cellpadding="3">
<tr><td><code>Space</code></td><td>Play / Stop</td></tr>
<tr><td><code>F5</code></td><td>Play Song from beginning</td></tr>
<tr><td><code>F6</code></td><td>Play current Pattern</td></tr>
<tr><td><code>F7</code></td><td>Play from current row</td></tr>
<tr><td><code>F8</code></td><td>Stop</td></tr>
<tr><td><code>R</code></td><td>Toggle Record mode</td></tr>
<tr><td><code>Esc</code></td><td>Toggle Record mode</td></tr>
<tr><td><code>`</code></td><td>Toggle Metronome</td></tr>
</table>

<h3>Note Entry (QWERTY layout)</h3>
<table width="100%" border="0" cellpadding="3">
<tr><td><code>Z S X D C V G B H N J M</code></td><td>C4&ndash;B4 (lower octave row)</td></tr>
<tr><td><code>Q 2 W 3 E R 5 T 6 Y 7 U I</code></td><td>C5&ndash;C6 (upper octave row)</td></tr>
<tr><td><code>[</code> / <code>]</code></td><td>Decrease / Increase base octave</td></tr>
<tr><td><code>1</code></td><td>Note Off</td></tr>
</table>

<h3>Navigation (Renoise / SoundTracker style)</h3>
<table width="100%" border="0" cellpadding="3">
<tr><td><code>Arrow Keys</code></td><td>Move cursor</td></tr>
<tr><td><code>Ctrl+Up / Down</code></td><td>Previous / Next order (song) position</td></tr>
<tr><td><code>Ctrl+Left / Right</code></td><td>Switch pattern in current order slot</td></tr>
<tr><td><code>Tab</code></td><td>Next track</td></tr>
<tr><td><code>Shift+Tab</code></td><td>Previous track</td></tr>
<tr><td><code>Home</code></td><td>Go to row 0</td></tr>
<tr><td><code>End</code></td><td>Go to last row</td></tr>
<tr><td><code>Page Up / Down</code></td><td>Scroll up / down by 16 rows</td></tr>
</table>

<h3>Selection</h3>
<table width="100%" border="0" cellpadding="3">
<tr><td><code>Shift+Arrows</code></td><td>Extend selection</td></tr>
<tr><td><code>Ctrl+A</code></td><td>Select all</td></tr>
<tr><td><i>Right-click &rarr; Select</i></td><td>Select sub-column or entire track</td></tr>
</table>

<h3>Editing</h3>
<table width="100%" border="0" cellpadding="3">
<tr><td><code>Ctrl+Z / Ctrl+Y</code></td><td>Undo / Redo</td></tr>
<tr><td><code>Ctrl+C / Ctrl+V / Ctrl+X</code></td><td>Copy / Paste / Cut (selection)</td></tr>
<tr><td><code>Delete</code></td><td>Clear cell or selection</td></tr>
<tr><td><code>Insert</code></td><td>Insert empty row, push rest down</td></tr>
<tr><td><code>Backspace</code></td><td>Delete row, pull rest up</td></tr>
</table>

<h3>Pattern Operations</h3>
<table width="100%" border="0" cellpadding="3">
<tr><td><code>Ctrl+Insert</code></td><td>Insert new pattern in order</td></tr>
<tr><td><code>Ctrl+Delete</code></td><td>Remove pattern from order</td></tr>
<tr><td><code>Ctrl+K</code></td><td>Duplicate pattern</td></tr>
<tr><td><code>F9</code></td><td>Jump to row 0</td></tr>
<tr><td><code>F10</code></td><td>Jump to row 16</td></tr>
<tr><td><code>F11</code></td><td>Jump to row 32</td></tr>
<tr><td><code>F12</code></td><td>Jump to row 48</td></tr>
</table>

<h3>Track Controls</h3>
<table width="100%" border="0" cellpadding="3">
<tr><td><code>\</code></td><td>Mute / Unmute current track</td></tr>
<tr><td><code>Ctrl+\</code></td><td>Solo / Unsolo current track</td></tr>
</table>

<h2>Tracker Right-Click Menu</h2>
<p>Right-click anywhere in the pattern grid to open the context menu.</p>
<table width="100%" border="0" cellpadding="4">
<tr><td valign="top"><b>Transpose &rarr; Semitone Up / Down</b></td><td>Shift notes by &plusmn;1 semitone. Operates on the current selection, or the whole track if nothing is selected.</td></tr>
<tr><td valign="top"><b>Transpose &rarr; Octave Up / Down</b></td><td>Shift notes by &plusmn;12 semitones.</td></tr>
<tr><td valign="top"><b>Select &rarr; Select this Sub-column</b></td><td>Select all rows of the current note sub-column.</td></tr>
<tr><td valign="top"><b>Select &rarr; Select Entire Track</b></td><td>Select all rows and fields of the current track.</td></tr>
</table>

<h2>Tracker Commands</h2>
<p>Enter commands in the <b>Eff</b> (effect) column (format: <code>CC PP</code> &mdash; command + parameter hex):</p>
<table width="100%" border="1" cellpadding="5">
<tr><th>Cmd</th><th>Description</th><th>Example</th></tr>
<tr><td><code>0A</code></td><td>Volume slide: upper nibble = up, lower = down</td><td><code>0A10</code> (slide up)</td></tr>
<tr><td><code>0C</code></td><td>Set volume (00&ndash;7F)</td><td><code>0C60</code></td></tr>
<tr><td><code>03</code></td><td>Portamento (slide pitch toward previous note)</td><td><code>0308</code></td></tr>
<tr><td><code>0E9</code></td><td>Retrigger note every N ticks</td><td><code>0E93</code></td></tr>
<tr><td><code>0EC</code></td><td>Note cut at tick N</td><td><code>0EC4</code></td></tr>
<tr><td><code>0F</code></td><td>Set speed (01&ndash;1F) or BPM (&ge;20)</td><td><code>0F06</code> (speed), <code>0F78</code> (120 BPM)</td></tr>
</table>

<h2>Instruments</h2>
<p>Select the instrument type per instrument slot in the <b>Instrument</b> tab. Instruments are shared across the project and assigned to tracks in the <b>Project</b> tab.</p>
<table width="100%" border="1" cellpadding="5">
<tr><th>Type</th><th>Description</th></tr>
<tr><td><b>Sampler</b></td><td>Polyphonic sample player. Supports WAV, FLAC, MP3. Per-sample ADSR, looping, pitch, stereo/mono modes. Recording input is available through JACK.</td></tr>
<tr><td><b>SoundFont</b></td><td>SF2/SF3 file via FluidSynth. Choose preset from bank browser.</td></tr>
<tr><td><b>SFZ</b></td><td>SFZ text-format multi-sample instrument. Parsed at load time. Path stored as absolute reference (not copied into project).</td></tr>
<tr><td><b>XRNI</b></td><td>Renoise instrument format (ZIP archive with Instrument.xml + samples). Loaded directly from file; path stored as absolute reference.</td></tr>
<tr><td><b>Plugin (DSSI/LV2)</b></td><td>External software synthesizer plugin.</td></tr>
<tr><td><b>MIDI</b></td><td>External MIDI device output. Set channel, program and optional audio return.</td></tr>
<tr><td><b>Voice</b></td><td>Text-to-speech synthesis (espeak-ng / Festival).</td></tr>
</table>

<h2>Sampler</h2>
<p>The Sampler editor is available when a <b>Sampler</b> instrument is selected in the <b>Instrument</b> tab.</p>
<ul>
<li><b>Sample list</b> &mdash; Add / remove samples. Click anywhere on a row to select it. Enter changes name when you press Enter or leave the field.</li>
<li><b>Waveform view</b> &mdash; Click to place cursor; drag to select a region. When a selection exists, clicking near an edge drags that edge to resize it. The scale bar at the top is not selectable.</li>
<li><b>Preview (Play button)</b> &mdash; Plays the selected sample (or the current selection if one exists) at original pitch. Press <b>Stop</b> to end playback.</li>
<li><b>Loop checkbox</b> &mdash; Loops the whole sample or selection during preview.</li>
<li><b>Use FX Chain checkbox</b> &mdash; Routes preview through the track&rsquo;s effect chain (only when the instrument is assigned to a track).</li>
<li><b>Record button</b> &mdash; Records from the configured JACK input. OSS currently provides playback output only. Overwrites the selected sample, or adds a new sample if none is selected. Press <b>Stop</b> to finish recording.</li>
<li><b>VU meters</b> &mdash; Show the live recording level during recording.</li>
</ul>

<h2>Project Tab</h2>
<p>Each track row contains:</p>
<ul>
<li><b>Name</b> &mdash; Track name (editable; press Enter to commit).</li>
<li><b>Instrument</b> &mdash; Assigned instrument (click to select).</li>
<li><b>Notation</b> &mdash; Preferred notation style for the Notation view (Violin, Bass, Violin+Bass, Drums). Active for SoundFont, SFZ, XRNI, Plugin and MIDI instruments.</li>
<li><b>Output Bus</b> &mdash; Route track output to Master or an auxiliary bus.</li>
<li><b>V&plusmn;</b> &mdash; Velocity humanization spread (0 = off). Applied at playback/render; tracker data is not altered. Active for SoundFont, SFZ, XRNI, Plugin and MIDI.</li>
<li><b>T ms</b> &mdash; Timing humanization: random onset delay in milliseconds. Same rules as velocity humanization.</li>
</ul>
<p>Use the <b>+</b> / <b>&minus;</b> buttons to add or remove tracks and buses.</p>

<h2>DSP Effects</h2>
<p>Add effects to any track or bus strip in the <b>Mixer</b> tab. Chains can be saved/loaded as <code>.chain</code> files.</p>
<table width="100%" border="1" cellpadding="3">
<tr><th>Effect</th><th>Description</th></tr>
<tr><td>Gain</td><td>Volume adjustment with presets</td></tr>
<tr><td>3-Band EQ</td><td>Low / Mid / High shelving equalizer</td></tr>
<tr><td>Graphical EQ</td><td>12-band parametric equalizer</td></tr>
<tr><td>Low-Pass Filter</td><td>Resonant low-pass filter</td></tr>
<tr><td>High-Pass Filter</td><td>Resonant high-pass filter</td></tr>
<tr><td>Delay</td><td>Echo with feedback and dry/wet mix</td></tr>
<tr><td>Reverb</td><td>Schroeder algorithmic reverb</td></tr>
<tr><td>Echo</td><td>Multi-tap echo with damping</td></tr>
<tr><td>Compressor</td><td>Dynamics compression (threshold, ratio, attack, release)</td></tr>
<tr><td>Limiter</td><td>Peak limiting with ceiling threshold</td></tr>
<tr><td>Gate</td><td>Noise gate</td></tr>
<tr><td>Phaser</td><td>All-pass modulation (rate, depth)</td></tr>
<tr><td>Flanger</td><td>Time modulation with feedback</td></tr>
<tr><td>Chorus</td><td>Stereo LFO modulation</td></tr>
<tr><td>Distortion</td><td>Soft-clipping saturation</td></tr>
<tr><td>Exciter</td><td>HPF + harmonic saturation</td></tr>
<tr><td>Cabinet</td><td>Guitar cabinet simulation (IR-based)</td></tr>
<tr><td>Stereo Expander</td><td>M/S width control</td></tr>
<tr><td>Ring Modulator</td><td>Sine-carrier ring modulation</td></tr>
</table>

<h2>Audio Engine</h2>
<p>Disgrace supports <b>JACK</b> and <b>OSS</b> audio backends. Use JACK for audio+MIDI ports and live recording inputs; use OSS for simple <code>/dev/dsp</code> playback.</p>
<ul>
<li>Sample rate: auto-detected from the active backend</li>
<li>Up to 32 audio input/output channels</li>
<li>Up to 32 auxiliary mixer buses</li>
<li>32 DSP slots per track or bus</li>
</ul>

<h2>File Formats</h2>
<table width="100%" border="0" cellpadding="4">
<tr><td><b>.dg</b></td><td>Disgrace project (tar.gz archive containing song.json and any embedded samples)</td></tr>
<tr><td><b>WAV / AIFF / FLAC</b></td><td>Sample audio (via libsndfile)</td></tr>
<tr><td><b>SF2 / SF3</b></td><td>SoundFont instrument (via FluidSynth)</td></tr>
<tr><td><b>.sfz</b></td><td>SFZ instrument (absolute path reference; not embedded)</td></tr>
<tr><td><b>.xrni</b></td><td>Renoise instrument (absolute path reference; not embedded)</td></tr>
<tr><td><b>.xrns</b></td><td>Renoise song (importable via File &rarr; Import XRNS)</td></tr>
<tr><td><b>.chain</b></td><td>Saved DSP effect chain (JSON)</td></tr>
<tr><td><b>.json</b></td><td>Individual effect preset</td></tr>
</table>
<p>When closing a project with unsaved changes, Disgrace will prompt you to save.</p>

<hr>
<p><i>Disgrace v0.1.0 &mdash; Built with wxWidgets, JACK and OSS support</i></p>
</body>
</html>
)";

    m_html_win->SetPage(html);
}

} // namespace disgrace_ns
