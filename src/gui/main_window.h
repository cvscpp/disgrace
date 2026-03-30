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

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Value_Slider.H>
#include "core/transport.h" // Add this line

namespace disgrace_ns
{

    class Engine;
    class TransportBar;
    class TrackerView;
    class Track; 
    class TrackerPanel;
    class TracksPanel;
    class NotationPanel;
    class InstrumentPanel;
    class MixerPanel;
    class SettingsPanel;
    class ProjectPanel;

    class MainWindow : public Fl_Double_Window
    {
    public:
        MainWindow(int w, int h, const char* title, Engine& engine);
        ~MainWindow(); // Destructor declaration
        Track& track(size_t index); // Now Track is declared

        // Static callback for timer
        static void timer_cb(void* data);
        static void request_update_cb(void* data);

        void update_all_uis();
        void request_update();

        // Standard FLTK event handler
        int handle(int event) override;

    private:
        Engine& m_engine;
        int m_cursor_row = 0; // Add this line

        TransportBar*    m_transport;
        Fl_Box*          m_status;
        Fl_Tabs* m_tabs;
        Fl_Group* m_project_tab;
        Fl_Group* m_tracker_tab;
        Fl_Group* m_tracks_tab;
        Fl_Group* m_notation_tab;
        Fl_Group* m_mixer_tab;
        Fl_Group* m_instrument_tab;
        Fl_Group* m_settings_tab;
        Fl_Group* m_help_tab;
        Fl_Check_Button* m_loop_btn;

        TrackerPanel* m_tracker_panel;
        TracksPanel* m_tracks_panel;
        NotationPanel* m_notation_panel;
        InstrumentPanel* m_instrument_panel;
        MixerPanel* m_mixer_panel;
        SettingsPanel* m_settings_panel;
        ProjectPanel* m_project_panel;

    private:
        void init_project_tab(int w, int h);
        void init_tracker_tab(int w, int h);
        void init_tracks_tab(int w, int h);
        void init_notation_tab(int w, int h);
        void init_instrument_tab(int w, int h);
        void init_mixer_tab(int w, int h);
        void init_settings_tab(int w, int h);
        void init_help_tab(int w, int h);
    };

} // namespace disgrace_ns
