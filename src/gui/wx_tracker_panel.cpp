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

#include "wx_tracker_panel.h"
#include "wx_tracker_view.h"
#include "wx_detached_frame.h"

#include <wx/artprov.h>
#include "../core/engine.h"
#include "../instrument/voice_instrument.h"

#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/dialog.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>

namespace disgrace_ns {

wxBEGIN_EVENT_TABLE(TrackerPanel, wxPanel)
    EVT_BUTTON(ID_ADD_PATTERN, TrackerPanel::on_add_pattern)
    EVT_BUTTON(ID_REMOVE_PATTERN, TrackerPanel::on_remove_pattern)
    EVT_BUTTON(ID_COPY_PATTERN, TrackerPanel::on_copy_pattern)
    EVT_BUTTON(ID_INC_PATTERN, TrackerPanel::on_inc_pattern)
    EVT_BUTTON(ID_DEC_PATTERN, TrackerPanel::on_dec_pattern)
    EVT_TOGGLEBUTTON(ID_FOLLOW_PLAYBACK, TrackerPanel::on_follow_playback)
    EVT_BUTTON(ID_DETACH, TrackerPanel::on_detach)
wxEND_EVENT_TABLE()

TrackerPanel::TrackerPanel(wxWindow* parent, Engine& engine)
    : wxPanel(parent, wxID_ANY), m_engine(engine)
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    const int BTN_H = 26;
    const int PAD   = 2;   // wxALL border per button
    const int M     = PAD * 2;  // total horizontal margin per button slot

    // ── Content row: [left column] [tracker view] ──────────────────────────
    wxBoxSizer* content_sizer = new wxBoxSizer(wxHORIZONTAL);

    // ── Left column: 3 button rows + pattern list ──────────────────────────
    wxBoxSizer* left_col = new wxBoxSizer(wxVERTICAL);

    // Pre-compute button widths so each row exactly matches pattern_list_width.
    //   Each slot = total / n_buttons; button pixel width = slot - M.
    const int PLW   = 120;      // pattern list width (matches wxSize below)
    const int B2W   = PLW / 3 - M;   // row-2: 3 equal buttons  → 36px
    const int B3W   = PLW / 2 - M;   // row-3: 2 equal buttons  → 56px
    const int B1FW  = PLW - (BTN_H + M) - M;  // row-1: follow fills rest → 86px

    // Row 1: Detach  |  Follow
    wxBoxSizer* row1 = new wxBoxSizer(wxHORIZONTAL);
    m_detach_btn = new wxButton(this, ID_DETACH, "", wxDefaultPosition, wxSize(BTN_H, BTN_H));
    m_detach_btn->SetMinSize(wxSize(BTN_H, BTN_H));
    m_detach_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_FULL_SCREEN, wxART_BUTTON, wxSize(14, 14)));
    m_detach_btn->SetToolTip("Detach / re-attach tracker view");
    row1->Add(m_detach_btn, 0, wxALL, PAD);
    m_follow_btn = new wxToggleButton(this, ID_FOLLOW_PLAYBACK, wxString::FromUTF8("↻"), wxDefaultPosition, wxSize(B1FW, BTN_H));
    m_follow_btn->SetMinSize(wxSize(B1FW, BTN_H));
    m_follow_btn->SetValue(true);
    m_follow_playback = true;
    m_follow_btn->SetToolTip("Follow playback position");
    row1->Add(m_follow_btn, 0, wxALL, PAD);
    left_col->Add(row1, 0, wxEXPAND);

    // Row 2: Add  |  Remove  |  Copy
    wxBoxSizer* row2 = new wxBoxSizer(wxHORIZONTAL);
    m_add_pattern_btn = new wxButton(this, ID_ADD_PATTERN, "", wxDefaultPosition, wxSize(B2W, BTN_H));
    m_add_pattern_btn->SetMinSize(wxSize(B2W, BTN_H));
    m_add_pattern_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_PLUS, wxART_BUTTON, wxSize(14, 14)));
    m_add_pattern_btn->SetToolTip("Add pattern to order");
    row2->Add(m_add_pattern_btn, 0, wxALL, PAD);
    m_remove_pattern_btn = new wxButton(this, ID_REMOVE_PATTERN, "", wxDefaultPosition, wxSize(B2W, BTN_H));
    m_remove_pattern_btn->SetMinSize(wxSize(B2W, BTN_H));
    m_remove_pattern_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_MINUS, wxART_BUTTON, wxSize(14, 14)));
    m_remove_pattern_btn->SetToolTip("Remove pattern from order");
    row2->Add(m_remove_pattern_btn, 0, wxALL, PAD);
    m_copy_pattern_btn = new wxButton(this, ID_COPY_PATTERN, "", wxDefaultPosition, wxSize(B2W, BTN_H));
    m_copy_pattern_btn->SetMinSize(wxSize(B2W, BTN_H));
    m_copy_pattern_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY, wxART_BUTTON, wxSize(14, 14)));
    m_copy_pattern_btn->SetToolTip("Duplicate pattern in order");
    row2->Add(m_copy_pattern_btn, 0, wxALL, PAD);
    left_col->Add(row2, 0, wxEXPAND);

    // Row 3: Prev position  |  Next position
    wxBoxSizer* row3 = new wxBoxSizer(wxHORIZONTAL);
    m_dec_pattern_btn = new wxButton(this, ID_DEC_PATTERN, "", wxDefaultPosition, wxSize(B3W, BTN_H));
    m_dec_pattern_btn->SetMinSize(wxSize(B3W, BTN_H));
    m_dec_pattern_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_BACK, wxART_BUTTON, wxSize(14, 14)));
    m_dec_pattern_btn->SetToolTip("Decrement pattern index at current order position");
    row3->Add(m_dec_pattern_btn, 0, wxALL, PAD);
    m_inc_pattern_btn = new wxButton(this, ID_INC_PATTERN, "", wxDefaultPosition, wxSize(B3W, BTN_H));
    m_inc_pattern_btn->SetMinSize(wxSize(B3W, BTN_H));
    m_inc_pattern_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_BUTTON, wxSize(14, 14)));
    m_inc_pattern_btn->SetToolTip("Increment pattern index at current order position");
    row3->Add(m_inc_pattern_btn, 0, wxALL, PAD);
    left_col->Add(row3, 0, wxEXPAND);

    // Pattern list scroll (expands vertically to fill remaining space)
    int pattern_list_width = 120;
    m_pattern_scroll = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition,
                                            wxSize(pattern_list_width, -1), wxVSCROLL);
    m_pattern_scroll->SetScrollRate(0, 25);
    m_pattern_list_container = new wxPanel(m_pattern_scroll, wxID_ANY);
    left_col->Add(m_pattern_scroll, 1, wxEXPAND | wxALL, 2);

    content_sizer->Add(left_col, 0, wxEXPAND | wxALL, 0);

    // Tracker view (fills remaining horizontal space)
    m_tracker = new TrackerView(this, wxID_ANY, m_engine.pattern(), m_engine);
    content_sizer->Add(m_tracker, 1, wxEXPAND | wxALL, 2);

    main_sizer->Add(content_sizer, 1, wxEXPAND | wxALL, 0);

    // Voice text editor (shown only for Voice instrument tracks)
    wxBoxSizer* voice_sizer = new wxBoxSizer(wxHORIZONTAL);
    voice_sizer->Add(new wxStaticText(this, wxID_ANY, "Text:"), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    m_voice_text_field = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 24));
    m_voice_text_field->Bind(wxEVT_TEXT, [this](wxCommandEvent& ev) {
        if (m_voice_text_field && m_last_voice_idx != 255) {
            // Get current track and instrument
            size_t current_track = m_engine.m_record_track;
            if (current_track < m_engine.track_count()) {
                auto* inst = m_engine.track(current_track).instrument();
                if (inst && inst->type() == InstrumentType::Voice) {
                    VoiceInstrument* voice = static_cast<VoiceInstrument*>(inst);
                    voice->set_text(m_voice_text_field->GetValue().ToStdString(), m_last_voice_idx);
                }
            }
        }
    });
    m_voice_text_field->Hide();
    voice_sizer->Add(m_voice_text_field, 1, wxEXPAND | wxALL, 2);
    
    // Copy/Paste/Search buttons
    m_voice_copy_btn = new wxButton(this, wxID_ANY, "Copy", wxDefaultPosition, wxSize(50, 24));
    m_voice_copy_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY, wxART_BUTTON, wxSize(14, 14)));
    m_voice_copy_btn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        if (m_last_voice_idx != 255) {
            size_t current_track = m_engine.m_record_track;
            if (current_track < m_engine.track_count()) {
                auto* inst = m_engine.track(current_track).instrument();
                if (inst && inst->type() == InstrumentType::Voice) {
                    VoiceInstrument* voice = static_cast<VoiceInstrument*>(inst);
                    m_voice_clipboard = voice->get_text(m_last_voice_idx);
                }
            }
        }
    });
    m_voice_copy_btn->Hide();
    voice_sizer->Add(m_voice_copy_btn, 0, wxALL, 2);
    
    m_voice_paste_btn = new wxButton(this, wxID_ANY, "Paste", wxDefaultPosition, wxSize(55, 24));
    m_voice_paste_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE, wxART_BUTTON, wxSize(14, 14)));
    m_voice_paste_btn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        if (m_last_voice_idx != 255 && !m_voice_clipboard.empty()) {
            size_t current_track = m_engine.m_record_track;
            if (current_track < m_engine.track_count()) {
                auto* inst = m_engine.track(current_track).instrument();
                if (inst && inst->type() == InstrumentType::Voice) {
                    VoiceInstrument* voice = static_cast<VoiceInstrument*>(inst);
                    voice->set_text(m_voice_clipboard, m_last_voice_idx);
                    m_voice_text_field->SetValue(wxString::FromUTF8(m_voice_clipboard));
                }
            }
        }
    });
    m_voice_paste_btn->Hide();
    voice_sizer->Add(m_voice_paste_btn, 0, wxALL, 2);
    
    m_voice_search_btn = new wxButton(this, wxID_ANY, "Find", wxDefaultPosition, wxSize(45, 24));
    m_voice_search_btn->SetBitmap(wxArtProvider::GetBitmap(wxART_FIND, wxART_BUTTON, wxSize(14, 14)));
    m_voice_search_btn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        wxString search = wxGetTextFromUser("Search for text:", "Find in Voice Instrument");
        if (!search.IsEmpty()) {
            m_voice_search_term = search.ToStdString();
            size_t current_track = m_engine.m_record_track;
            if (current_track < m_engine.track_count()) {
                auto* inst = m_engine.track(current_track).instrument();
                if (inst && inst->type() == InstrumentType::Voice) {
                    VoiceInstrument* voice = static_cast<VoiceInstrument*>(inst);
                    // Find next occurrence in phrases (0-255)
                    uint8_t start_idx = (m_last_voice_idx == 255) ? 0 : (m_last_voice_idx + 1) % 256;
                    for (int i = 0; i < 256; ++i) {
                        uint8_t idx = (uint8_t)((start_idx + i) % 256);
                        std::string text = voice->get_text(idx);
                        if (text.find(m_voice_search_term) != std::string::npos) {
                            m_last_voice_idx = idx;
                            m_voice_text_field->SetValue(wxString::FromUTF8(text));
                            wxMessageBox(wxString::Format("Found '%s' at phrase index %d", search, idx), "Found");
                            return;
                        }
                    }
                    wxMessageBox("Text not found", "Not Found");
                }
            }
        }
    });
    m_voice_search_btn->Hide();
    voice_sizer->Add(m_voice_search_btn, 0, wxALL, 2);
    
    main_sizer->Add(voice_sizer, 0, wxEXPAND | wxALL, 0);

    SetSizer(main_sizer);

    update_pattern_list();
}

void TrackerPanel::on_add_pattern(wxCommandEvent& event) {
    size_t new_pos = m_engine.add_pattern_to_order();
    m_selected_order_idx = (int)new_pos;
    m_engine.m_edit_order_pos.store(new_pos);
    m_engine.set_active_pattern(m_engine.order_list()[new_pos]);
    update_pattern_list();
}

void TrackerPanel::on_remove_pattern(wxCommandEvent& event) {
    const auto& order = m_engine.order_list();
    if (m_selected_order_idx >= 0 && (size_t)m_selected_order_idx < order.size()) {
        m_engine.remove_pattern_from_order((size_t)m_selected_order_idx);
        if (m_selected_order_idx >= (int)m_engine.order_list().size()) {
            m_selected_order_idx = (int)m_engine.order_list().size() - 1;
        }
        if (m_selected_order_idx < 0) m_selected_order_idx = 0;
        
        if (!m_engine.order_list().empty()) {
            m_engine.m_edit_order_pos.store((size_t)m_selected_order_idx);
            m_engine.set_active_pattern(m_engine.order_list()[m_selected_order_idx]);
        }
        update_pattern_list();
    }
}

void TrackerPanel::on_copy_pattern(wxCommandEvent& event) {
    const auto& order = m_engine.order_list();
    if (m_selected_order_idx >= 0 && (size_t)m_selected_order_idx < order.size()) {
        size_t new_pos = m_engine.copy_pattern_in_order((size_t)m_selected_order_idx);
        m_selected_order_idx = (int)new_pos;
        m_engine.m_edit_order_pos.store(new_pos);
        m_engine.set_active_pattern(m_engine.order_list()[new_pos]);
        update_pattern_list();
    }
}

void TrackerPanel::on_inc_pattern(wxCommandEvent& event) {
    size_t pos = m_selected_order_idx;
    auto order = m_engine.order_list();
    if (pos < order.size()) {
        if (order[pos] < m_engine.pattern_count() - 1) {
            order[pos]++;
            m_engine.set_order(order);
            m_engine.set_active_pattern(order[pos]);
            m_tracker->set_pattern(m_engine.pattern());
            update_pattern_list();
        }
    }
}

void TrackerPanel::on_dec_pattern(wxCommandEvent& event) {
    size_t pos = m_selected_order_idx;
    auto order = m_engine.order_list();
    if (pos < order.size()) {
        if (order[pos] > 0) {
            order[pos]--;
            m_engine.set_order(order);
            m_engine.set_active_pattern(order[pos]);
            m_tracker->set_pattern(m_engine.pattern());
            update_pattern_list();
        }
    }
}

void TrackerPanel::on_follow_playback(wxCommandEvent& event) {
    m_follow_playback = m_follow_btn->GetValue();
}

void TrackerPanel::on_detach(wxCommandEvent& event) {
    if (m_detached_frame) {
        return;
    }
    Hide();
    m_detached_frame = new DetachedFrame(this, "Tracker", GetParent(), m_tab_index);
    m_detached_frame->set_on_detach_callback([this]() { m_detached_frame = nullptr; });
}

void TrackerPanel::update_pattern_list() {
    m_tracker->set_pattern(m_engine.pattern());
    const auto& order = m_engine.order_list();
    m_last_order_size = order.size();
    m_last_pattern_count = m_engine.pattern_count();

    m_pattern_list_container->DestroyChildren();
    m_pattern_list_container->SetBackgroundColour(ThemeManager::toWxColour(m_engine.m_tracker_bg));
    
    m_order_buttons.clear();
    m_pattern_length_inputs.clear();

    int row_h = 25;
    int start_x = 0;
    int start_y = 0;

    for (size_t i = 0; i < order.size(); ++i) {
        int cur_y = start_y + (int)(i * row_h);

        wxString pos_str;
        pos_str.Printf("%02zu:", i);
        wxButton* b = new wxButton(m_pattern_list_container, wxID_ANY, pos_str, wxPoint(start_x, cur_y), wxSize(35, row_h));
        if (m_selected_order_idx == (int)i) {
            b->SetBackgroundColour(ThemeManager::toWxColour(m_engine.m_selection_color));
            b->SetForegroundColour(ThemeManager::contrastColor(m_engine.m_selection_color));
        } else {
            b->SetBackgroundColour(ThemeManager::toWxColour(m_engine.m_bg_color));
            b->SetForegroundColour(ThemeManager::toWxColour(m_engine.m_fg_color));
        }
        b->Bind(wxEVT_BUTTON, [this, i](wxCommandEvent& ev) {
            auto& eng = this->m_engine;
            auto ord = eng.order_list();
            if (i < ord.size()) {
                eng.m_edit_order_pos.store(i);
                eng.set_active_pattern(ord[i]);
                this->m_selected_order_idx = (int)i;
                this->m_tracker->set_pattern(eng.pattern());
                this->update_pattern_list();
            }
        });
        m_order_buttons[i] = b;

        wxString pat_str;
        pat_str.Printf("%02zu", order[i]);
        wxStaticText* p = new wxStaticText(m_pattern_list_container, wxID_ANY, pat_str, wxPoint(start_x + 40, cur_y + 5), wxSize(25, row_h - 5));
        p->SetForegroundColour(ThemeManager::toWxColour(m_engine.m_tracker_note));

        int pat_idx = order[i];
        size_t len = m_engine.pattern(pat_idx).row_count();
        wxString len_str;
        len_str.Printf("%zu", len);
        wxTextCtrl* len_inp = new wxTextCtrl(m_pattern_list_container, wxID_ANY, len_str, wxPoint(start_x + 70, cur_y + 2), wxSize(40, 20), wxTE_PROCESS_ENTER);
        len_inp->SetBackgroundColour(ThemeManager::toWxColour(m_engine.m_tracker_bg));
        len_inp->SetForegroundColour(ThemeManager::toWxColour(m_engine.m_tracker_text));
        
        len_inp->Bind(wxEVT_TEXT_ENTER, [this, pat_idx](wxCommandEvent& ev) {
            wxTextCtrl* inp = dynamic_cast<wxTextCtrl*>(ev.GetEventObject());
            if (inp) {
                long new_len = 0;
                if (inp->GetValue().ToLong(&new_len) && new_len > 0 && new_len <= 512) {
                    this->m_engine.resize_pattern(pat_idx, (size_t)new_len);
                    inp->SetValue(wxString::Format("%zu", this->m_engine.pattern(pat_idx).row_count()));
                    this->m_tracker->recalculate_size();
                    this->m_tracker->Refresh();
                }
            }
        });
        m_pattern_length_inputs[pat_idx] = len_inp;
    }

    m_pattern_list_container->SetSize(wxSize(120, (int)(order.size() * row_h)));
    m_pattern_scroll->SetVirtualSize(120, (int)(order.size() * row_h));

    m_pattern_scroll->Refresh();
}

void TrackerPanel::update() {
    if (m_tracker) {
        if (m_follow_playback) {
            size_t engine_order_pos = m_engine.m_order_pos.load();
            if (m_engine.m_edit_order_pos.load() != engine_order_pos) {
                m_engine.m_edit_order_pos.store(engine_order_pos);
                m_engine.set_active_pattern(m_engine.m_order[engine_order_pos]);
                m_tracker->set_pattern(m_engine.pattern());
            }
        }

        size_t current_edit_pos = m_engine.m_edit_order_pos.load();
        size_t current_order_size = m_engine.order_list().size();
        size_t current_pattern_count = m_engine.pattern_count();

        if ((int)current_edit_pos != m_selected_order_idx ||
            current_order_size != m_last_order_size ||
            current_pattern_count != m_last_pattern_count) {

            m_selected_order_idx = (int)current_edit_pos;
            update_pattern_list();
        }

        if (m_engine.transport_state() != TransportState::Stopped) {
            m_tracker->ensure_cursor_visible();
        }
        m_tracker->Refresh();
        update_voice_text_field();  // Update voice text field based on cursor position
    }
}

void TrackerPanel::grab_focus() {
    if (m_tracker) {
        m_tracker->SetFocus();
    }
}

int TrackerPanel::get_cursor_row() const {
    if (m_tracker) return m_tracker->get_cursor_row();
    return 0;
}

void TrackerPanel::handle_note_action(Action action) {
    if (m_tracker) {
        m_tracker->handle_action(action);
    }
}

void TrackerPanel::update_voice_text_field() {
    if (!m_tracker || !m_voice_text_field) return;
    
    size_t current_track = m_tracker->get_cursor_track();
    int cursor_row = m_tracker->get_cursor_row();
    int cursor_col = m_tracker->get_cursor_col();
    
    // Check if current track uses Voice instrument
    if (current_track < m_engine.track_count()) {
        auto* inst = m_engine.track(current_track).instrument();
        if (inst && inst->type() == InstrumentType::Voice) {
            VoiceInstrument* voice = static_cast<VoiceInstrument*>(inst);
            
            // Get phrase index from current tracker event's sample_idx
            const auto& ev = m_engine.pattern().event(current_track, cursor_row, cursor_col);
            uint8_t phrase_idx = ev.sample_idx;
            
            std::string text = voice->get_text(phrase_idx);
            if (m_voice_text_field->GetValue() != wxString::FromUTF8(text)) {
                m_voice_text_field->SetValue(wxString::FromUTF8(text));
            }
            m_last_voice_idx = phrase_idx;
            m_voice_text_field->Show();
            m_voice_copy_btn->Show();
            m_voice_paste_btn->Show();
            m_voice_search_btn->Show();
            m_voice_text_field->GetParent()->Layout();
            return;
        }
    }
    
    // Hide field and buttons if not a Voice track
    m_voice_text_field->Hide();
    m_voice_copy_btn->Hide();
    m_voice_paste_btn->Hide();
    m_voice_search_btn->Hide();
    m_last_voice_idx = 255;
    m_voice_text_field->GetParent()->Layout();
}

} // namespace disgrace_ns
