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

#pragma once
#include <wx/dialog.h>
#include <wx/choice.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>
#include <wx/sizer.h>
#include <vector>
#include "../io/tab_exporter.h"

namespace disgrace_ns {

class Engine;

// Modal dialog: pick a track, guitar/bass with a tuning preset, and an
// open-string note per subtrack (one string per tracker column). After
// ShowModal() == wxID_OK, call get_config().
class TabExportDialog : public wxDialog {
public:
    TabExportDialog(wxWindow* parent, Engine& engine);

    TabExportConfig get_config() const;

private:
    void on_instrument(wxCommandEvent& event);
    void on_preset(wxCommandEvent& event);
    void apply_preset();
    void rebuild_string_rows();

    Engine& m_engine;
    size_t m_cols; // subtracks == strings

    wxChoice*   m_track_choice;
    wxRadioButton* m_guitar_radio;
    wxRadioButton* m_bass_radio;
    wxChoice*   m_preset_choice;
    wxStaticBoxSizer* m_string_rows_box;
    std::vector<wxComboBox*> m_string_notes;
    std::vector<int> m_string_midi; // last good value per combo
};

} // namespace disgrace_ns
