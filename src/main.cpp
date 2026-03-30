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

#include <wx/wxprec.h>
#include <wx/app.h>
#include <wx/msgdlg.h>

#include "gui/wx_main_window.h"
#include "core/engine.h"

namespace disgrace_ns {
class DisgraceApp : public wxApp {
public:
    virtual bool OnInit() override;

private:
    Engine* m_engine = nullptr;
    WxMainWindow* m_window = nullptr;
};

wxIMPLEMENT_APP(DisgraceApp);

bool DisgraceApp::OnInit() {
    if (!wxApp::OnInit())
        return false;

    try {
        m_engine = new Engine();

        if (!m_engine->initialize()) {
            delete m_engine;
            return false;
        }
        m_window = new WxMainWindow(1280, 800, wxString::FromUTF8("Disgrace"), *m_engine);
        m_window->Show(true);

        return true;
    }
    catch (const std::exception& e) {
        wxMessageBox(wxString::Format("Fatal error: %s", e.what()), "Error", wxOK | wxICON_ERROR);
        return false;
    }
}

} // namespace disgrace_ns

int main(int argc, char** argv) {
    return wxEntry(argc, argv);
}
