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

#include "wx_startup_dialog.h"
#include "../core/app_info.h"

#include <wx/artprov.h>
#include <wx/button.h>
#include <wx/filename.h>
#include <wx/font.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/stdpaths.h>
#include <filesystem>
#include <optional>

namespace disgrace_ns {

namespace {

std::optional<wxImage> load_logo_image() {
    namespace fs = std::filesystem;

    const fs::path exe_path(wxStandardPaths::Get().GetExecutablePath().ToStdString());
    const fs::path exe_dir = exe_path.parent_path();
    const fs::path resources_dir(wxStandardPaths::Get().GetResourcesDir().ToStdString());
    const fs::path cwd = fs::current_path();
    const fs::path logo_name = fs::path("imgs") / "disgrace.png";

    const fs::path candidates[] = {
        cwd / logo_name,
        exe_dir / logo_name,
        exe_dir / ".." / logo_name,
        exe_dir / ".." / "share" / "disgrace" / logo_name,
        resources_dir / logo_name,
        fs::path("/usr/local/share/disgrace") / logo_name,
        fs::path("/usr/share/disgrace") / logo_name,
    };

    for (const fs::path& candidate : candidates) {
        const fs::path normalized = candidate.lexically_normal();
        if (!fs::exists(normalized)) {
            continue;
        }

        wxImage image;
        if (image.LoadFile(wxString::FromUTF8(normalized.string()), wxBITMAP_TYPE_PNG)) {
            return image;
        }
    }

    return std::nullopt;
}

wxBitmap scaled_logo_bitmap(const wxImage& image, int width, int height) {
    wxImage scaled = image.Copy();
    scaled.Rescale(width, height, wxIMAGE_QUALITY_HIGH);
    return wxBitmap(scaled);
}

} // namespace

StartupDialog::StartupDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY,
               wxString::FromUTF8(app_display_name_with_version()),
               wxDefaultPosition, wxDefaultSize,
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    SetMinSize(wxSize(560, 420));

    wxBoxSizer* top = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* header = new wxBoxSizer(wxHORIZONTAL);

    if (std::optional<wxImage> logo_image = load_logo_image(); logo_image && logo_image->IsOk()) {
        header->Add(new wxStaticBitmap(this, wxID_ANY, scaled_logo_bitmap(*logo_image, 128, 128)),
                    0, wxALIGN_TOP | wxRIGHT, 16);
    } else {
        header->Add(new wxStaticBitmap(this, wxID_ANY,
                                       wxArtProvider::GetBitmap(wxART_INFORMATION, wxART_OTHER, wxSize(96, 96))),
                    0, wxALIGN_TOP | wxRIGHT, 16);
    }

    wxBoxSizer* title_box = new wxBoxSizer(wxVERTICAL);

    wxStaticText* title = new wxStaticText(this, wxID_ANY, wxString::FromUTF8(app_display_name()));
    wxFont title_font = title->GetFont();
    title_font.SetPointSize(title_font.GetPointSize() + 8);
    title_font.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(title_font);
    title_box->Add(title, 0, wxBOTTOM, 6);

    wxStaticText* version = new wxStaticText(this, wxID_ANY,
                                             "Version " + wxString::FromUTF8(app_version()));
    wxFont version_font = version->GetFont();
    version_font.SetWeight(wxFONTWEIGHT_BOLD);
    version->SetFont(version_font);
    title_box->Add(version, 0, wxBOTTOM, 10);

    wxStaticText* subtitle = new wxStaticText(
        this, wxID_ANY,
        "Minimalist tracker DAW\nKeyboard-driven composition, notation, and audio editing.");
    subtitle->Wrap(340);
    title_box->Add(subtitle, 0, wxBOTTOM, 4);

    header->Add(title_box, 1, wxEXPAND);
    top->Add(header, 0, wxEXPAND | wxALL, 16);
    top->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 16);

    wxBoxSizer* body = new wxBoxSizer(wxVERTICAL);

    wxStaticText* license_hdr = new wxStaticText(this, wxID_ANY, "License");
    wxFont section_font = license_hdr->GetFont();
    section_font.SetWeight(wxFONTWEIGHT_BOLD);
    license_hdr->SetFont(section_font);
    body->Add(license_hdr, 0, wxTOP | wxBOTTOM, 10);

    wxStaticText* license_text = new wxStaticText(
        this, wxID_ANY,
        wxString::FromUTF8(app_license_summary()) +
            "\nYou may copy, modify, and redistribute this software under the terms of the GPL.");
    license_text->Wrap(500);
    body->Add(license_text, 0, wxBOTTOM, 14);

    wxStaticText* disclaimer_hdr = new wxStaticText(this, wxID_ANY, "Disclaimer");
    disclaimer_hdr->SetFont(section_font);
    body->Add(disclaimer_hdr, 0, wxBOTTOM, 10);

    wxStaticText* disclaimer_text = new wxStaticText(
        this, wxID_ANY, wxString::FromUTF8(app_disclaimer_summary()));
    disclaimer_text->Wrap(500);
    body->Add(disclaimer_text, 0, wxBOTTOM, 10);

    top->Add(body, 1, wxEXPAND | wxLEFT | wxRIGHT, 16);
    top->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT, 16);

    wxBoxSizer* button_row = new wxBoxSizer(wxHORIZONTAL);
    button_row->AddStretchSpacer();
    wxButton* continue_btn = new wxButton(this, wxID_OK, "Continue");
    continue_btn->SetDefault();
    button_row->Add(continue_btn, 0, wxTOP | wxBOTTOM, 12);
    top->Add(button_row, 0, wxEXPAND | wxLEFT | wxRIGHT, 16);

    SetSizerAndFit(top);
    CentreOnScreen();
}

} // namespace disgrace_ns
