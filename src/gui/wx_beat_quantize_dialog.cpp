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

#include "wx_beat_quantize_dialog.h"
#include "../core/engine.h"
#include "../instrument/sample_instrument.h"
#include "../mixer/track.h"
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/statbox.h>

namespace disgrace_ns {

enum {
    ID_BQ_DETECT = wxID_HIGHEST + 200,
    ID_BQ_METRO_RADIO,
    ID_BQ_TRACK_RADIO,
    ID_BQ_TRACK_CHOICE,
};

wxBEGIN_EVENT_TABLE(BeatQuantizeDialog, wxDialog)
    EVT_BUTTON(ID_BQ_DETECT, BeatQuantizeDialog::on_detect)
    EVT_BUTTON(wxID_OK,       BeatQuantizeDialog::on_apply)
    EVT_RADIOBUTTON(ID_BQ_METRO_RADIO, BeatQuantizeDialog::on_ref_changed)
    EVT_RADIOBUTTON(ID_BQ_TRACK_RADIO, BeatQuantizeDialog::on_ref_changed)
    EVT_CHOICE(ID_BQ_TRACK_CHOICE,     BeatQuantizeDialog::on_ref_changed)
wxEND_EVENT_TABLE()

BeatQuantizeDialog::BeatQuantizeDialog(wxWindow* parent, Engine& engine,
                                       std::shared_ptr<SampleData> sample_data,
                                       int sel_track)
    : wxDialog(parent, wxID_ANY, "Beat Quantize",
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , m_engine(engine)
    , m_source(sample_data)
    , m_sel_track(sel_track)
{
    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);

    // ---- Sensitivity row ----
    wxBoxSizer* sens_row = new wxBoxSizer(wxHORIZONTAL);
    sens_row->Add(new wxStaticText(this, wxID_ANY, "Onset sensitivity:"), 0,
                  wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    m_sensitivity_slider = new wxSlider(this, wxID_ANY, 35, 1, 100,
                                        wxDefaultPosition, wxSize(200, -1));
    sens_row->Add(m_sensitivity_slider, 1, wxEXPAND);
    top->Add(sens_row, 0, wxEXPAND | wxALL, 8);

    // ---- Quantize-to row ----
    wxStaticBox* ref_box = new wxStaticBox(this, wxID_ANY, "Quantize to:");
    wxStaticBoxSizer* ref_sizer = new wxStaticBoxSizer(ref_box, wxVERTICAL);

    wxString metro_label = wxString::Format("Metronome  (BPM: %.1f)", engine.tempo());
    m_metro_radio = new wxRadioButton(this, ID_BQ_METRO_RADIO, metro_label,
                                      wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    ref_sizer->Add(m_metro_radio, 0, wxALL, 4);

    wxBoxSizer* track_row = new wxBoxSizer(wxHORIZONTAL);
    m_track_radio = new wxRadioButton(this, ID_BQ_TRACK_RADIO, "Track:");
    track_row->Add(m_track_radio, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);

    m_track_choice = new wxChoice(this, ID_BQ_TRACK_CHOICE);
    for (int i = 0; i < (int)engine.track_count(); ++i) {
        if (i == sel_track) continue;
        auto* inst = engine.track(i).instrument();
        auto* sampler = dynamic_cast<SampleInstrument*>(inst);
        if (sampler && sampler->sample_count() > 0) {
            m_track_choice->Append(engine.track(i).name(), reinterpret_cast<void*>((intptr_t)i));
        }
    }
    if (m_track_choice->GetCount() == 0)
        m_track_choice->Append("<no sampler tracks>", reinterpret_cast<void*>((intptr_t)-1));
    m_track_choice->SetSelection(0);
    m_track_choice->Enable(false);
    track_row->Add(m_track_choice, 1, wxEXPAND);

    ref_sizer->Add(track_row, 0, wxEXPAND | wxALL, 4);
    top->Add(ref_sizer, 0, wxEXPAND | wxALL, 8);

    // ---- Strength row ----
    wxBoxSizer* str_row = new wxBoxSizer(wxHORIZONTAL);
    str_row->Add(new wxStaticText(this, wxID_ANY, "Strength:"), 0,
                 wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
    m_strength_slider = new wxSlider(this, wxID_ANY, 100, 0, 100,
                                     wxDefaultPosition, wxSize(200, -1));
    str_row->Add(m_strength_slider, 1, wxEXPAND);
    top->Add(str_row, 0, wxEXPAND | wxALL, 8);

    // ---- Onsets label ----
    m_onsets_label = new wxStaticText(this, wxID_ANY,
                                      "Click 'Detect Onsets' first");
    top->Add(m_onsets_label, 0, wxALL, 8);

    // ---- Button row ----
    wxBoxSizer* btn_row = new wxBoxSizer(wxHORIZONTAL);
    btn_row->Add(new wxButton(this, ID_BQ_DETECT, "Detect Onsets"), 0, wxRIGHT, 8);
    btn_row->AddStretchSpacer();
    btn_row->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxRIGHT, 4);
    btn_row->Add(new wxButton(this, wxID_OK, "Apply"), 0);
    top->Add(btn_row, 0, wxEXPAND | wxALL, 8);

    SetSizerAndFit(top);
    Centre();

    // Select metro by default
    m_metro_radio->SetValue(true);
}

void BeatQuantizeDialog::on_ref_changed(wxCommandEvent&)
{
    bool track_mode = m_track_radio->GetValue();
    m_track_choice->Enable(track_mode);
    rebuild_target_grid();
}

void BeatQuantizeDialog::do_detect()
{
    float threshold = m_sensitivity_slider->GetValue() / 100.0f;
    OnsetDetector det(threshold, 80.0f);
    m_source_onsets = det.detect(*m_source);
    m_onsets_label->SetLabel(wxString::Format("Detected %zu onset(s)", m_source_onsets.size()));
    rebuild_target_grid();
}

void BeatQuantizeDialog::on_detect(wxCommandEvent&)
{
    do_detect();
}

void BeatQuantizeDialog::rebuild_target_grid()
{
    if (m_metro_radio->GetValue()) {
        m_target_grid = BeatQuantizer::make_metro_grid(
            (float)m_engine.tempo(),
            m_source->sample_rate,
            (int)m_source->left.size());
    } else {
        // Get selected reference track
        int sel = m_track_choice->GetSelection();
        if (sel == wxNOT_FOUND) return;
        intptr_t track_idx = reinterpret_cast<intptr_t>(m_track_choice->GetClientData(sel));
        if (track_idx < 0 || (size_t)track_idx >= m_engine.track_count()) return;

        auto* inst = m_engine.track((int)track_idx).instrument();
        auto* sampler = dynamic_cast<SampleInstrument*>(inst);
        if (!sampler || sampler->sample_count() == 0) return;

        auto ref_data = sampler->get_sample(0).data;
        if (!ref_data || ref_data->left.empty()) return;

        float threshold = m_sensitivity_slider->GetValue() / 100.0f;
        OnsetDetector det(threshold, 80.0f);
        auto ref_onsets = det.detect(*ref_data);

        m_target_grid = BeatQuantizer::make_track_grid(ref_onsets, ref_data->sample_rate);
        // Add sentinel at source length
        m_target_grid.beats.push_back((int)m_source->left.size());
    }
}

void BeatQuantizeDialog::on_apply(wxCommandEvent&)
{
    if (m_source_onsets.empty())
        do_detect();

    if (m_target_grid.beats.size() < 2) {
        wxMessageBox("Could not build target grid. Check settings.", "Beat Quantize",
                     wxOK | wxICON_WARNING, this);
        return;
    }

    float strength = m_strength_slider->GetValue() / 100.0f;
    m_result = BeatQuantizer::quantize(*m_source, m_source_onsets, m_target_grid, strength);
    EndModal(wxID_OK);
}

} // namespace disgrace_ns
