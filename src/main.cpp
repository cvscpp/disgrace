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
#include <wx/strconv.h>
#include <wx/wxcrtbase.h>
#include <locale.h>

#include "gui/wx_main_window.h"
#include "core/engine.h"

namespace disgrace_ns {
class DisgraceApp : public wxApp {
public:
    virtual bool OnInit() override;

    // wxApp::SetCLocale() resets to the "C" locale for number formatting.
    // On FreeBSD this breaks wxString's wchar_t ↔ multibyte conversions
    // because "C" only covers ASCII.  Override to keep the UTF-8 locale we
    // established in main().
    virtual void SetCLocale() override {}

    // Force UTF-8 early.
    virtual bool Initialize(int& argc, wxChar** argv) override {
        setenv("LANG", "C.UTF-8", 1);
        setenv("LC_ALL", "C.UTF-8", 1);
        setlocale(LC_ALL, "C.UTF-8");
        
        bool ok = wxApp::Initialize(argc, argv);
        
        wxUpdateLocaleIsUtf8();
        wxConvCurrent = &wxConvUTF8;
        return ok;
    }

private:
    Engine* m_engine = nullptr;
    WxMainWindow* m_window = nullptr;
};

wxIMPLEMENT_APP(DisgraceApp);

bool DisgraceApp::OnInit() {
    if (!wxApp::OnInit())
        return false;

    try {
        wxInitAllImageHandlers();
        m_engine = new Engine();

        if (!m_engine->initialize()) {
            delete m_engine;
            return false;
        }
        m_window = new WxMainWindow(1280, 800, "Disgrace", *m_engine);
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
    // Aggressive locale forcing before wxWidgets takes over.
    setenv("LANG", "C.UTF-8", 1);
    setenv("LC_ALL", "C.UTF-8", 1);
    setlocale(LC_ALL, "C.UTF-8");
    
    // On some FreeBSD systems, library version mismatches between GCC and Clang
    // can cause crashes in std::string/wxString. Using the system compiler (clang++)
    // is highly recommended.
    
    return wxEntry(argc, argv);
}
