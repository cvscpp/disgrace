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

// Central last-used-directory handling for all file dialogs, so they reopen
// where the user last worked instead of at the executable's directory.
//
// Categories in use: "project" (.dg load/save), "import" (audio/xrns/midi),
// "export" (wav/lilypond), "instrument" (.dgi), "sample" (wav/...),
// "soundfont" (.sf2/.sf3/.sfz/.xrni), "model" (TTS onnx), "chain"
// (.chain), "reference" (reference tracks), "settings" (.json).

#include <string>
#include <wx/string.h>
#include <wx/filename.h>
#include <wx/utils.h>

#include "../core/config_manager.h"

namespace disgrace_ns
{

// Resolve the directory a dialog should start in:
// stored last-used dir (if it still exists) -> caller fallback (usually the
// project folder, if it exists) -> home directory -> "" (wxWidgets default).
inline wxString file_history_initial_dir(const std::string& category,
                                         const wxString& fallback_dir = wxEmptyString)
{
    const auto& all = ConfigManager::instance().config().last_dirs;
    auto it = all.find(category);
    if (it != all.end() && !it->second.empty()) {
        wxFileName dir(wxString::FromUTF8(it->second.c_str()));
        if (dir.DirExists())
            return dir.GetFullPath();
    }
    if (!fallback_dir.empty()) {
        wxFileName dir(fallback_dir);
        if (dir.DirExists())
            return dir.GetFullPath();
    }
    if (wxDirExists(wxGetHomeDir()))
        return wxGetHomeDir();
    return wxEmptyString;
}

// Same, but derives the fallback from the engine's current project file.
template <typename EngineT>
wxString file_history_project_dir(EngineT& engine, const std::string& category)
{
    wxString fallback;
    if (!engine.project_path().empty()) {
        wxFileName f(wxString::FromUTF8(engine.project_path().c_str()));
        wxString dir = f.GetPath();
        if (!dir.empty())
            fallback = dir;
    }
    return file_history_initial_dir(category, fallback);
}

// Remember the directory of a successfully chosen path (file or folder).
// Only the in-memory config is touched here; it is persisted on shutdown
// via the existing save_config() path.
inline void file_history_remember(const std::string& category, const wxString& path)
{
    wxFileName f(path);
    wxString dir;
    if (wxDirExists(path)) {
        dir = path;
    } else {
        dir = f.GetPath();
    }
    if (dir.empty())
        return;
    ConfigManager::instance().config().last_dirs[category] = dir.ToStdString();
}

} // namespace disgrace_ns
