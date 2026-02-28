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
    class TransportPanel;
    class TrackerView;
    class Track; // Added forward declaration for Track
    class TrackerPanel;
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

        void update_all_uis();

        // Standard FLTK event handler
        int handle(int event) override;

    private:
        Engine& m_engine;
        int m_cursor_row = 0; // Add this line

        TransportPanel*  m_transport;
        Fl_Box*          m_status;
        Fl_Tabs* m_tabs;
        Fl_Group* m_project_tab;
        Fl_Group* m_tracker_tab;
        Fl_Group* m_mixer_tab;
        Fl_Group* m_instrument_tab;
        Fl_Group* m_settings_tab;
        Fl_Check_Button* m_loop_btn;

        TrackerPanel* m_tracker_panel;
        InstrumentPanel* m_instrument_panel;
        MixerPanel* m_mixer_panel;
        SettingsPanel* m_settings_panel;
        ProjectPanel* m_project_panel;

    private:
        void init_project_tab(int w, int h);
        void init_tracker_tab(int w, int h);
        void init_instrument_tab(int w, int h);
        void init_mixer_tab(int w, int h);
        void init_settings_tab(int w, int h);
    };

} // namespace disgrace_ns
