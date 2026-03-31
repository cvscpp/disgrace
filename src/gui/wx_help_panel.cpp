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
<h1>Disgrace - Digital Audio Workstation</h1>

<h2>Interface</h2>
<p>Disgrace is a pattern-based music sequencer inspired by trackers like Renoise and OpenMPT.</p>
<ul>
<li><b>Project</b> - Manage tracks and metadata</li>
<li><b>Tracker</b> - Pattern editing</li>
<li><b>Tracks</b> - Arrange patterns on a timeline</li>
<li><b>Notation</b> - Piano roll view</li>
<li><b>Instrument</b> - Manage instruments and samples</li>
<li><b>Mixer</b> - Audio mixing and effects</li>
<li><b>Settings</b> - Configure audio, MIDI, and GUI</li>
<li><b>Help</b> - This documentation</li>
</ul>

<h2>Keyboard Shortcuts</h2>

<h3>Transport</h3>
<table width="100%" border="0">
<tr><td><code>Space</code></td><td>Play / Stop</td></tr>
<tr><td><code>F5</code></td><td>Play Song</td></tr>
<tr><td><code>F6</code></td><td>Play Pattern</td></tr>
<tr><td><code>F7</code></td><td>Play from Current Row</td></tr>
<tr><td><code>F8</code></td><td>Stop</td></tr>
<tr><td><code>R</code></td><td>Toggle Record Mode</td></tr>
<tr><td><code>Esc</code></td><td>Toggle Record Mode</td></tr>
<tr><td><code>`</code></td><td>Toggle Metronome</td></tr>
</table>

<h3>Note Entry (QWERTY)</h3>
<table width="100%" border="0">
<tr><td><code>Z S X D C V G B H N J M</code></td><td>C4 - B4</td></tr>
<tr><td><code>Q 2 W 3 E R 5 T 6 Y 7 U I</code></td><td>C5 - C6</td></tr>
<tr><td><code>[ / ]</code></td><td>Decrease / Increase Octave</td></tr>
<tr><td><code>^ or `</code></td><td>Note Off</td></tr>
</table>

<h3>Navigation</h3>
<table width="100%" border="0">
<tr><td><code>Arrow Keys</code></td><td>Move Cursor</td></tr>
<tr><td><code>Ctrl+Up/Down</code></td><td>Previous / Next Order Position</td></tr>
<tr><td><code>Ctrl+Left/Right</code></td><td>Decrease / Increase Pattern Index</td></tr>
<tr><td><code>TAB</code></td><td>Jump to Next Column</td></tr>
<tr><td><code>Shift+TAB</code></td><td>Jump to Previous Column</td></tr>
</table>

<h3>Editing</h3>
<table width="100%" border="0">
<tr><td><code>Ctrl+Z</code></td><td>Undo</td></tr>
<tr><td><code>Ctrl+Y</code></td><td>Redo</td></tr>
<tr><td><code>Ctrl+C</code></td><td>Copy</td></tr>
<tr><td><code>Ctrl+V</code></td><td>Paste</td></tr>
<tr><td><code>Ctrl+X</code></td><td>Cut</td></tr>
<tr><td><code>Delete</code></td><td>Clear</td></tr>
<tr><td><code>Ctrl+A</code></td><td>Select All</td></tr>
</table>

<h3>Pattern Operations</h3>
<table width="100%" border="0">
<tr><td><code>Insert</code></td><td>Insert Row</td></tr>
<tr><td><code>Backspace</code></td><td>Delete Row</td></tr>
<tr><td><code>Ctrl+Insert</code></td><td>Insert Pattern</td></tr>
<tr><td><code>Ctrl+Delete</code></td><td>Delete Pattern</td></tr>
<tr><td><code>Ctrl+K</code></td><td>Duplicate Pattern</td></tr>
<tr><td><code>F9</code></td><td>Jump to Row 0</td></tr>
<tr><td><code>F10</code></td><td>Jump to Row 16</td></tr>
<tr><td><code>F11</code></td><td>Jump to Row 32</td></tr>
<tr><td><code>F12</code></td><td>Jump to Row 48</td></tr>
</table>

<h3>Track Controls</h3>
<table width="100%" border="0">
<tr><td><code>\</code></td><td>Mute Track</td></tr>
<tr><td><code>Ctrl+\</code></td><td>Solo Track</td></tr>
</table>

<h2>Tracker Commands</h2>
<p>Enter commands in the Effect column (XXY format, where Y is the parameter):</p>
<table width="100%" border="1" cellpadding="5">
<tr><th>Cmd</th><th>Description</th><th>Example</th></tr>
<tr><td><code>0A</code></td><td>Volume Slide Up/Down</td><td>0A01 (slide up)</td></tr>
<tr><td><code>0C</code></td><td>Set Volume (0-7F)</td><td>0C60</td></tr>
<tr><td><code>0E9</code></td><td>Retrig Note (N ticks)</td><td>0E903</td></tr>
<tr><td><code>0EC</code></td><td>Note Cut (at tick N)</td><td>0EC4</td></tr>
<tr><td><code>03</code></td><td>Portamento</td><td>0308</td></tr>
<tr><td><code>0F</code></td><td>Set Tempo/BPM</td><td>0F06 (speed) / 0F80 (120 BPM)</td></tr>
</table>

<h2>Effects</h2>
<p>The Mixer supports the following effects (insert on tracks or buses):</p>
<table width="100%" border="1" cellpadding="3">
<tr><th>Effect</th><th>Description</th></tr>
<tr><td>Gain</td><td>Volume adjustment with presets</td></tr>
<tr><td>Delay</td><td>Echo with feedback and mix</td></tr>
<tr><td>Reverb</td><td>Schroeder reverb algorithm</td></tr>
<tr><td>Echo</td><td>Delayed feedback with damping</td></tr>
<tr><td>Limiter</td><td>Peak limiting with threshold</td></tr>
<tr><td>Compressor</td><td>Dynamics compression</td></tr>
<tr><td>Gate</td><td>Noise gate</td></tr>
<tr><td>Graphical EQ</td><td>12-band equalizer</td></tr>
<tr><td>Phaser</td><td>All-pass modulation</td></tr>
<tr><td>Flanger</td><td>Time modulation with feedback</td></tr>
<tr><td>Chorus</td><td>Stereo LFO modulation</td></tr>
<tr><td>Distortion</td><td>Soft-clipping saturation</td></tr>
<tr><td>Exciter</td><td>HPF + harmonic saturation</td></tr>
<tr><td>Cabinet</td><td>Guitar cabinet simulation</td></tr>
<tr><td>Stereo Expander</td><td>M/S width control</td></tr>
<tr><td>Ring Modulator</td><td>Sine carrier modulation</td></tr>
</table>

<h2>Instruments</h2>
<p>Disgrace supports multiple instrument types:</p>
<ul>
<li><b>Sampler</b> - WAV, FLAC, MP3 sample playback with ADSR envelope</li>
<li><b>SoundFont</b> - SF2/SF3 file support via FluidSynth</li>
<li><b>DSSI</b> - Linux plugin support</li>
<li><b>MIDI</b> - External MIDI device output</li>
</ul>

<h2>Audio Engine</h2>
<p>Disgrace uses JACK audio for low-latency I/O. Make sure JACK is running before starting.</p>
<ul>
<li>Sample rate: 44100 Hz</li>
<li>Up to 32 audio input/output channels</li>
<li>Up to 32 auxiliary buses</li>
<li>32 effect slots per track/bus</li>
</ul>

<h2>File Formats</h2>
<ul>
<li><b>Projects:</b> .disgrace (JSON + archive)</li>
<li><b>Samples:</b> WAV, AIFF, FLAC (via libsndfile)</li>
<li><b>Instruments:</b> SF2, SF3 (SoundFont)</li>
<li><b>Effect Chains:</b> .chain (JSON)</li>
<li><b>Effect Presets:</b> .json</li>
</ul>

<hr>
<p><i>Disgrace v0.1.0 | Built with wxWidgets</i></p>
</body>
</html>
)";

    m_html_win->SetPage(html);
}

} // namespace disgrace_ns
