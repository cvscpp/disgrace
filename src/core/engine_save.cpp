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

#include "engine.h"
#include "../io/song_serializer.h"
#include "../io/project_archive.h"
#include "../io/xrns_importer.h"
#include <filesystem>
#include <iostream>
#include <chrono>

namespace fs = std::filesystem;

namespace disgrace_ns
{

void Engine::save_project(const ::std::string& path)
{
    try {
        fs::path tmp_dir = fs::temp_directory_path() / "disgrace_save_tmp";
        if (fs::exists(tmp_dir)) fs::remove_all(tmp_dir);
        fs::create_directories(tmp_dir);

        if (SongSerializer::save(*this, tmp_dir.string())) {
            if (ProjectArchive::save(path, tmp_dir.string())) {
                std::cout << "Project saved successfully to " << path << std::endl;
            } else {
                std::cerr << "Failed to archive project to " << path << std::endl;
            }
        } else {
            std::cerr << "Failed to serialize song to temporary directory" << std::endl;
        }

        fs::remove_all(tmp_dir);
    } catch (const std::exception& e) {
        std::cerr << "Exception during save: " << e.what() << std::endl;
    }
}

void Engine::load_project(const ::std::string& path)
{
    try {
        fs::path tmp_base = fs::temp_directory_path() / "disgrace_load";
        fs::create_directories(tmp_base);
        
        // Create a unique temporary directory for this load session
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        fs::path tmp_dir = tmp_base / std::to_string(now);
        
        if (fs::exists(tmp_dir)) fs::remove_all(tmp_dir);
        fs::create_directories(tmp_dir);

        if (ProjectArchive::extract(path, tmp_dir.string())) {
            if (SongSerializer::load(*this, tmp_dir.string())) {
                m_project_temp_dir = tmp_dir.string();
                std::cout << "Project loaded successfully from " << path << std::endl;
            } else {
                std::cerr << "Failed to deserialize song from " << path << std::endl;
                fs::remove_all(tmp_dir);
            }
        } else {
            std::cerr << "Failed to extract project archive from " << path << std::endl;
            fs::remove_all(tmp_dir);
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception during load: " << e.what() << std::endl;
    }
}

void Engine::import_audio(const ::std::string& path)
{
    std::string path_str(path);
    
    if (path_str.size() >= 5 && path_str.compare(path_str.size() - 5, 5, ".xrns") == 0) {
        if (XrnsImporter::import(*this, path)) {
            std::cout << "XRNS import successful" << std::endl;
        } else {
            std::cerr << "Failed to import XRNS file" << std::endl;
        }
        return;
    }
    
    std::cout << "Import audio: " << path << std::endl;
}

} // namespace disgrace_ns
