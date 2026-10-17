/*
 * Disgrace - Digital Audio Workstation
 * Copyright (C) 2025  Miroslav Shaltev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "wx_tab_export_dialog.h"
#include "../core/engine.h"
#include "../sequencer/pattern.h"
#include "../mixer/track.h"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <algorithm>
#include <cctype>
#include <cstring>

namespace disgrace_ns {

namespace {
    struct TuningPreset {
        const char* name;
        std::vector<int> notes; // column order: low string first
    };

    const TuningPreset kGuitarPresets[] = {
        {"Standard",        {40, 45, 50, 55, 59, 64}},       // E2 A2 D3 G3 B3 E4
        {"Drop D",          {38, 45, 50, 55, 59, 64}},       // D2 A2 D3 G3 B3 E4
        {"Half Step Down",  {39, 44, 49, 54, 58, 63}},       // Eb2 Bb2 Eb3 Ab3 C4 Eb4
        {"Open G",          {38, 43, 50, 55, 59, 62}},       // D2 G2 D3 G3 B3 D4
        {"Open D",          {38, 45, 50, 54, 57, 62}},       // D2 A2 D3 F#3 A3 D4
        {"Drop C",          {36, 43, 48, 55, 60, 64}},       // C2 G2 C3 G3 C4 E4
    };

    const TuningPreset kBassPresets[] = {
        {"Standard",        {28, 33, 38, 43}},               // E1 A1 D2 G2
        {"5-String",        {23, 28, 33, 38, 43}},           // B0 E1 A1 D2 G2
        {"Drop D",          {26, 33, 38, 43}},               // D1 A1 D2 G2
    };

    const int kNoteMin = 24; // C1
    const int kNoteMax = 96; // C8

    // Lenient note-name parser for the open-string combos: accepts the
    // display form "E-2"/"F#3" as well as typed "e2", "f#3", "eb2", "bb1".
    // Returns -1 when the text is not a note in [kNoteMin, kNoteMax].
    int parse_note_name(const wxString& text) {
        wxString ws(text);
        ws.Trim(true).Trim(false).Lower();
        std::string s = ws.ToStdString();
        size_t i = 0;
        if (i >= s.size()) return -1;
        static const char* letters = "cdefgab";
        static const int semis[] = {0, 2, 4, 5, 7, 9, 11};
        const char* p = std::strchr(letters, s[i]);
        if (!p) return -1;
        int semi = semis[p - letters];
        ++i;
        if (i < s.size() && s[i] == '#') { ++semi; ++i; }
        else if (i + 1 < s.size() && s[i] == 'b' && (std::isdigit((unsigned char)s[i + 1]) || s[i + 1] == '-')) {
            --semi; ++i; // flat, but not a bare "b3" (B natural)
        }
        if (i < s.size() && s[i] == '-') ++i;
        if (i >= s.size() || !std::isdigit((unsigned char)s[i])) return -1;
        int octave = 0;
        while (i < s.size() && std::isdigit((unsigned char)s[i])) {
            octave = octave * 10 + (s[i] - '0');
            ++i;
        }
        if (i != s.size()) return -1;
        const int midi = (octave + 1) * 12 + semi;
        return (midi >= kNoteMin && midi <= kNoteMax) ? midi : -1;
    }
}

TabExportDialog::TabExportDialog(wxWindow* parent, Engine& engine)
    : wxDialog(parent, wxID_ANY, "Export Guitar Tabs", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
      m_engine(engine)
{
    m_cols = engine.pattern().column_count(engine.m_record_track.load());

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);

    // Track
    wxBoxSizer* track_row = new wxBoxSizer(wxHORIZONTAL);
    track_row->Add(new wxStaticText(this, wxID_ANY, "Track:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    m_track_choice = new wxChoice(this, wxID_ANY);
    size_t cursor = engine.m_record_track.load();
    for (size_t t = 0; t < engine.track_count(); ++t) {
        const auto& tr = engine.track(t);
        size_t cols = engine.pattern().column_count(t);
        m_track_choice->Append(wxString::Format("%02zu: %s (%zu strings)", t + 1,
                                                tr.name().empty() ? "Unnamed" : tr.name(), cols));
    }
    if (cursor < engine.track_count()) m_track_choice->SetSelection((int)cursor);
    track_row->Add(m_track_choice, 1, wxEXPAND);
    top->Add(track_row, 0, wxEXPAND | wxALL, 8);

    // Instrument type
    wxBoxSizer* inst_row = new wxBoxSizer(wxHORIZONTAL);
    m_guitar_radio = new wxRadioButton(this, wxID_ANY, "Guitar", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    m_bass_radio = new wxRadioButton(this, wxID_ANY, "Bass");
    inst_row->Add(m_guitar_radio, 0, wxRIGHT, 12);
    inst_row->Add(m_bass_radio, 0);
    top->Add(inst_row, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);

    // Tuning preset
    wxBoxSizer* preset_row = new wxBoxSizer(wxHORIZONTAL);
    preset_row->Add(new wxStaticText(this, wxID_ANY, "Tuning:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    m_preset_choice = new wxChoice(this, wxID_ANY);
    for (const auto& p : kGuitarPresets) m_preset_choice->Append(p.name);
    m_preset_choice->SetSelection(0);
    preset_row->Add(m_preset_choice, 1, wxEXPAND);
    top->Add(preset_row, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    // Per-string open notes (one row per subtrack)
    wxStaticBoxSizer* strings_box = new wxStaticBoxSizer(wxVERTICAL, this, "Strings (open notes, one per subtrack)");
    top->Add(strings_box, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
    m_string_rows_box = strings_box;
    rebuild_string_rows();
    apply_preset();

    // OK / Cancel
    wxBoxSizer* btn_row = new wxBoxSizer(wxHORIZONTAL);
    btn_row->Add(CreateButtonSizer(wxOK | wxCANCEL), 1, wxEXPAND);
    top->Add(btn_row, 0, wxEXPAND | wxALL, 8);

    SetSizerAndFit(top);

    m_track_choice->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) {
        size_t sel = (size_t)m_track_choice->GetSelection();
        if (sel < m_engine.track_count()) {
            m_cols = m_engine.pattern().column_count(sel);
            if (m_cols == 0) m_cols = 1;
            rebuild_string_rows();
            apply_preset();
        }
    });
    m_guitar_radio->Bind(wxEVT_RADIOBUTTON, &TabExportDialog::on_instrument, this);
    m_bass_radio->Bind(wxEVT_RADIOBUTTON, &TabExportDialog::on_instrument, this);
    m_preset_choice->Bind(wxEVT_CHOICE, &TabExportDialog::on_preset, this);
}

void TabExportDialog::rebuild_string_rows() {
    for (auto* c : m_string_notes) c->Destroy();
    m_string_notes.clear();
    m_string_midi.clear();

    for (size_t i = 0; i < m_cols; ++i) {
        wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
        row->Add(new wxStaticText(this, wxID_ANY, wxString::Format("Subtrack %zu:", i + 1)),
                 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
        wxComboBox* combo = new wxComboBox(this, wxID_ANY, "", wxDefaultPosition,
                                           wxSize(90, -1), wxCB_DROPDOWN);
        for (int n = kNoteMin; n <= kNoteMax; ++n) combo->Append(TabExporter::note_name(n));
        combo->Bind(wxEVT_COMBOBOX, [this, i](wxCommandEvent&) {
            const int midi = parse_note_name(m_string_notes[i]->GetValue());
            if (midi >= 0) m_string_midi[i] = midi;
        });
        row->Add(combo, 0, wxALIGN_CENTER_VERTICAL);
        m_string_notes.push_back(combo);
        m_string_midi.push_back(kNoteMin);
        m_string_rows_box->Add(row, 0, wxALL, 2);
    }
    m_string_rows_box->Layout();
    Fit();
}

void TabExportDialog::apply_preset() {
    const bool bass = m_bass_radio->GetValue();
    const TuningPreset* presets = bass ? kBassPresets : kGuitarPresets;
    const size_t n_presets = bass ? sizeof(kBassPresets) / sizeof(kBassPresets[0])
                                  : sizeof(kGuitarPresets) / sizeof(kGuitarPresets[0]);
    int sel = m_preset_choice->GetSelection();
    if (sel < 0 || (size_t)sel >= n_presets) sel = 0;
    const auto& notes = presets[sel].notes;

    for (size_t c = 0; c < m_string_notes.size(); ++c) {
        int midi;
        if (c < notes.size()) {
            midi = notes[c];
        } else {
            // More subtracks than preset strings: continue downwards in
            // perfect fourths below the lowest preset string.
            midi = notes.front() - (int)(c - notes.size() + 1) * 5;
        }
        midi = std::max(kNoteMin, std::min(kNoteMax, midi));
        m_string_notes[c]->SetValue(TabExporter::note_name(midi));
        m_string_midi[c] = midi;
    }
}

void TabExportDialog::on_instrument(wxCommandEvent& event) {
    const bool bass = m_bass_radio->GetValue();
    m_preset_choice->Clear();
    if (bass) {
        for (const auto& p : kBassPresets) m_preset_choice->Append(p.name);
    } else {
        for (const auto& p : kGuitarPresets) m_preset_choice->Append(p.name);
    }
    m_preset_choice->SetSelection(0);
    apply_preset();
}

void TabExportDialog::on_preset(wxCommandEvent& event) {
    apply_preset();
}

TabExportConfig TabExportDialog::get_config() const {
    TabExportConfig cfg;
    cfg.track = (size_t)m_track_choice->GetSelection();
    cfg.is_bass = m_bass_radio->GetValue();
    for (size_t i = 0; i < m_string_notes.size(); ++i) {
        // Accept typed "e2"/"f#3"/"eb2" as well as the display form; an
        // unparseable entry keeps its last good value.
        int midi = parse_note_name(m_string_notes[i]->GetValue());
        if (midi < 0) midi = m_string_midi[i];
        cfg.open_strings.push_back(midi);
    }
    return cfg;
}

} // namespace disgrace_ns
