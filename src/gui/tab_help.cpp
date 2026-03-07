#include "main_window.h"
#include <FL/Fl_Help_View.H>

namespace disgrace_ns {

void MainWindow::init_help_tab(int w, int h) {
    m_help_tab = new Fl_Group(0, 65, w, h - 100, "Help");
    m_help_tab->begin();
    
    Fl_Help_View* help = new Fl_Help_View(10, 75, w - 20, h - 120);
    help->value(
        "<html><body>"
        "<h1>Disgrace - Manual</h1>"
        "<h2>Overview</h2>"
        "<p>Disgrace is a minimalist tracker DAW. It uses a keyboard-driven workflow for composing music using patterns and samples/instruments.</p>"

        "<h2>Navigation</h2>"
        "<ul>"
        "<li><b>Project:</b> Manage tracks and pattern order.</li>"
        "<li><b>Tracker:</b> The main composing area. Edit notes and effects here.</li>"
        "<li><b>Mixer:</b> Control volume, pan, and effects for each track.</li>"
        "<li><b>Instruments:</b> Load and edit samples or SoundFonts.</li>"
        "<li><b>Settings:</b> Audio/MIDI config and Keyboard Layout selection.</li>"
        "</ul>"

        "<h2>Keyboard Shortcuts (Global)</h2>"
        "<ul>"
        "<li><b>Space:</b> Play / Stop</li>"
        "<li><b>R:</b> Toggle Record (Edit Mode)</li>"
        "<li><b>M:</b> Toggle Metronome</li>"
        "<li><b>F5:</b> Play Song</li>"
        "<li><b>F6:</b> Play Pattern</li>"
        "<li><b>F7:</b> Play from current row</li>"
        "<li><b>F8:</b> Stop</li>"
        "</ul>"

        "<h2>Tracker Editing</h2>"
        "<h3>Note Entry</h3>"
        "<p>In Record Mode (R), use your keyboard to input notes:</p>"
        "<ul>"
        "<li><b>Bottom Row (C4-B4):</b> Z S X D C V G B H N J M</li>"
        "<li><b>Top Row (C5-C6):</b> Q 2 W 3 E R 5 T 6 Y 7 U I</li>"
        "<li><b>Note Off:</b> ^ or ` (Backtick) - Inserts '===' to stop the current note.</li>"
        "<li><b>Delete/Backspace:</b> Clears the current field and moves up/down.</li>"
        "</ul>"
        "<p><i>Note: The Z/Y key mapping depends on the Layout setting (QWERTY/QWERTZ) in Settings.</i></p>"

        "<h3>Navigation & Selection</h3>"
        "<ul>"
        "<li><b>Arrow Keys:</b> Move cursor between fields, rows, and tracks.</li>"
        "<li><b>Page Up/Down:</b> Jump 16 rows.</li>"
        "<li><b>Home/End:</b> Jump to first/last row.</li>"
        "<li><b>Shift + Arrows:</b> Select a block of data.</li>"
        "<li><b>Esc:</b> Clear selection.</li>"
        "</ul>"

        "<h3>Columns & Effects</h3>"
        "<p>Each track can have multiple <b>note columns</b> (sub-tracks). Notes in the same column cut each other off (monophonic per column), while notes in different columns can overlap.</p>"
        "<ul>"
        "<li><b>Note field:</b> Entry of notes (C-4, D#5, etc.)</li>"
        "<li><b>Sample field:</b> Hex index of the sample to trigger.</li>"
        "<li><b>Volume field:</b> Hex value (00-7F) for note velocity.</li>"
        "<li><b>FX / Param fields:</b> Hex values for tracker effects (slides, arpeggios, etc.)</li>"
        "</ul>"

        "<h2>Instruments</h2>"
        "<h3>Sample Instrument</h3>"
        "<p>Load WAV/FLAC/MP3 files. Supports basic ADSR envelope and playback parameters.</p>"
        "<h3>SoundFont (SF2/SF3)</h3>"
        "<p>Load standard SoundFont files. The selected patch is automatically applied to all channels used by the tracker's columns.</p>"

        "<h2>Mixer</h2>"
        "<p>Each track has its own channel strip with volume, pan, mute, and solo controls. VU meters show real-time levels.</p>"

        "</body></html>"
    );
    
    m_help_tab->end();
}

} // namespace disgrace_ns
