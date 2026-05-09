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
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/radiobut.h>
#include <memory>
#include "../analysis/onset_detector.h"
#include "../audio/beat_quantizer.h"

namespace disgrace_ns {

class Engine;
class SampleData;

class BeatQuantizeDialog : public wxDialog {
public:
    // sample_data: the sample region to quantize (already extracted)
    // After ShowModal() == wxID_OK, call get_result() for the warped audio
    BeatQuantizeDialog(wxWindow* parent, Engine& engine,
                       std::shared_ptr<SampleData> sample_data,
                       int sel_track);

    std::shared_ptr<SampleData> get_result() const { return m_result; }

private:
    void on_detect(wxCommandEvent& event);
    void on_apply(wxCommandEvent& event);
    void on_ref_changed(wxCommandEvent& event);
    void do_detect();
    void rebuild_target_grid();

    Engine& m_engine;
    std::shared_ptr<SampleData> m_source;
    std::shared_ptr<SampleData> m_result;
    int m_sel_track;

    std::vector<Onset> m_source_onsets;
    BeatGrid m_target_grid;

    wxSlider*      m_sensitivity_slider;
    wxSlider*      m_strength_slider;
    wxRadioButton* m_metro_radio;
    wxRadioButton* m_track_radio;
    wxChoice*      m_track_choice;
    wxStaticText*  m_onsets_label;

    wxDECLARE_EVENT_TABLE();
};

} // namespace disgrace_ns
