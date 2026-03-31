#include "wx_help_panel.h"
#include "../core/engine.h"

#include <wx/sizer.h>
#include <wx/stattext.h>

namespace disgrace_ns {

HelpPanel::HelpPanel(wxWindow* parent, Engine& engine)
    : wxPanel(parent, wxID_ANY), m_engine(engine), m_html_win(nullptr)
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    m_html_win = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO);
    int sizes[] = {10, 12, 14, 16, 20, 20};
    m_html_win->SetFonts(wxEmptyString, wxEmptyString, sizes);

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

<h2>Getting Started</h2>
<p>Disgrace is a pattern-based music sequencer inspired by trackers like Renoise and OpenMPT.</p>
<p>The interface is organized into tabs:</p>
<ul>
<li><b>Project</b> - Manage tracks and metadata</li>
<li><b>Tracker</b> - Pattern editing with step sequencer</li>
<li><b>Tracks</b> - Arrange patterns on a timeline</li>
<li><b>Notation</b> - Piano roll view</li>
<li><b>Instrument</b> - Manage instruments and samples</li>
<li><b>Mixer</b> - Audio mixing and effects</li>
<li><b>Settings</b> - Configure audio, MIDI, and GUI</li>
<li><b>Help</b> - This documentation</li>
</ul>

<h2>Keyboard Shortcuts</h2>

<h3>Playback</h3>
<table width="100%" border="0">
<tr><td><code>Space</code></td><td>Play/Stop</td></tr>
<tr><td><code>Enter</code></td><td>Play Song</td></tr>
<tr><td><code>P</code></td><td>Play Pattern</td></tr>
<tr><td><code>R</code></td><td>Toggle Record</td></tr>
</table>

<h3>Navigation</h3>
<table width="100%" border="0">
<tr><td><code>[</code></td><td>Previous Order Position</td></tr>
<tr><td><code>]</code></td><td>Next Order Position</td></tr>
<tr><td><code>Tab</code></td><td>Next Tab</td></tr>
</table>

<h3>Editing</h3>
<table width="100%" border="0">
<tr><td><code>Ctrl+Z</code></td><td>Undo</td></tr>
<tr><td><code>Ctrl+Y</code></td><td>Redo</td></tr>
<tr><td><code>Ctrl+C</code></td><td>Copy</td></tr>
<tr><td><code>Ctrl+V</code></td><td>Paste</td></tr>
<tr><td><code>Ctrl+X</code></td><td>Cut</td></tr>
</table>

<h3>Track Controls</h3>
<table width="100%" border="0">
<tr><td><code>M</code></td><td>Mute Track</td></tr>
<tr><td><code>S</code></td><td>Solo Track</td></tr>
<tr><td><code>O</code></td><td>Octave Up</td></tr>
<tr><td><code>I</code></td><td>Octave Down</td></tr>
</table>

<h2>Tracker Commands</h2>
<p>Commands are entered in the Effect column (XXY format):</p>
<table width="100%" border="1" cellpadding="5">
<tr><th>Command</th><th>Description</th><th>Example</th></tr>
<tr><td><code>00</code></td><td>Set Volume</td><td>000-0FF</td></tr>
<tr><td><code>10</code></td><td>Set Panning</td><td>10-40</td></tr>
<tr><td><code>11</code></td><td>Pitch Up</td><td>1101</td></tr>
<tr><td><code>12</code></td><td>Pitch Down</td><td>1201</td></tr>
<tr><td><code>13</code></td><td>Portamento Up</td><td>130F</td></tr>
<tr><td><code>14</code></td><td>Portamento Down</td><td>140F</td></tr>
<tr><td><code>15</code></td><td>Tone Portamento</td><td>1500</td></tr>
<tr><td><code>19</code></td><td>Sample Offset</td><td>1900</td></tr>
<tr><td><code>1A</code></td><td>Delay</td><td>1A04</td></tr>
<tr><td><code>1B</code></td><td>Reverb</td><td>1B20</td></tr>
<tr><td><code>1C</code></td><td>Set Filter Cutoff</td><td>1C80</td></tr>
<tr><td><code>1D</code></td><td>Filter Resonance</td><td>1D40</td></tr>
<tr><td><code>1E</code></td><td>Arpeggio</td><td>1E0246</td></tr>
</table>

<h2>Audio Engine</h2>
<p>Disgrace uses JACK audio for low-latency audio I/O. Make sure JACK is running before starting the application.</p>
<h3>Supported Formats</h3>
<ul>
<li>WAV, AIFF, FLAC (via libsndfile)</li>
<li>SF2, GIG (SoundFont and GIG instruments)</li>
<li>FluidSynth for MIDI playback</li>
</ul>

<h2>MIDI Learn</h2>
<p>MIDI controller support is available through JACK MIDI. Connect your MIDI device in your JACK patchbay.</p>

<hr>
<p><i>Disgrace v0.1.0 | Built with wxWidgets</i></p>
</body>
</html>
)";

    m_html_win->SetPage(html);
}

} // namespace disgrace_ns
