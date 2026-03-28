#pragma once

#include <wx/wxprec.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/tglbtn.h>
#include <wx/stattext.h>
#include <unordered_map>

#include "../core/key_bindings.h"

enum {
    ID_ADD_PATTERN = 100,
    ID_REMOVE_PATTERN,
    ID_COPY_PATTERN,
    ID_DEC_PATTERN,
    ID_INC_PATTERN,
    ID_FOLLOW_PLAYBACK,
    ID_DETACH
};

namespace disgrace_ns {

class Engine;
class TrackerView;

class TrackerPanel : public wxPanel {
public:
    TrackerPanel(wxWindow* parent, Engine& engine);

    void update_pattern_list();
    void update();
    void grab_focus();
    int get_cursor_row() const;
    TrackerView* get_tracker_view() const { return m_tracker; }
    void handle_note_action(Action action);

    void on_add_pattern(wxCommandEvent& event);
    void on_remove_pattern(wxCommandEvent& event);
    void on_copy_pattern(wxCommandEvent& event);
    void on_inc_pattern(wxCommandEvent& event);
    void on_dec_pattern(wxCommandEvent& event);
    void on_follow_playback(wxCommandEvent& event);
    void on_detach(wxCommandEvent& event);

public:
    void set_tab_index(int idx) { m_tab_index = idx; }

private:
    void on_pattern_select(wxCommandEvent& event);
    void on_pattern_length(wxCommandEvent& event);

    Engine& m_engine;
    TrackerView* m_tracker;
    wxScrolledWindow* m_main_scroll;
    wxScrolledWindow* m_pattern_scroll;
    wxPanel* m_pattern_list_container;

    wxButton* m_add_pattern_btn;
    wxButton* m_remove_pattern_btn;
    wxButton* m_copy_pattern_btn;
    wxButton* m_dec_pattern_btn;
    wxButton* m_inc_pattern_btn;
    wxToggleButton* m_follow_btn;
    wxButton* m_detach_btn;

    int m_selected_order_idx = 0;
    int m_tab_index = -1;
    size_t m_last_order_size = 0;
    size_t m_last_pattern_count = 0;
    bool m_follow_playback = false;
    std::unordered_map<long, wxWindow*> m_pattern_length_inputs;
    std::unordered_map<long, wxButton*> m_order_buttons;
    class DetachedFrame* m_detached_frame = nullptr;

    wxDECLARE_EVENT_TABLE();
};

} // namespace disgrace_ns
