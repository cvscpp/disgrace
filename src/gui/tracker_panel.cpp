#include "tracker_panel.h"
#include "tracker_view.h"
#include "detached_window.h"
#include "main_window.h"
#include "../core/engine.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Int_Input.H>
#include <cstdlib>
#include <cstdint>
#include <utility> // For std::pair

namespace disgrace_ns {

TrackerPanel::TrackerPanel(int x, int y, int w, int h, Engine& engine)
    : Fl_Group(x, y, w, h), m_engine(engine) {
    
    begin();

    int btn_h = 25;
    int btn_y_offset = 0;

    m_detach_btn = new Fl_Button(x + w - 30, y + btn_y_offset, 30, 20, "[]");
    m_detach_btn->callback(cb_detach, this);

    m_add_pattern_btn = new Fl_Button(x, y + btn_y_offset, 40, btn_h, "Add");
    m_add_pattern_btn->labelsize(10);
    m_add_pattern_btn->callback(cb_add_pattern, this);
    
    m_remove_pattern_btn = new Fl_Button(x + 40, y + btn_y_offset, 40, btn_h, "Rem");
    m_remove_pattern_btn->labelsize(10);
    m_remove_pattern_btn->callback(cb_remove_pattern, this);
    
    m_copy_pattern_btn = new Fl_Button(x + 80, y + btn_y_offset, 40, btn_h, "Copy");
    m_copy_pattern_btn->labelsize(10);
    m_copy_pattern_btn->callback(cb_copy_pattern, this);

    m_dec_pattern_btn = new Fl_Button(x, y + btn_y_offset + btn_h, 60, btn_h, "-");
    m_dec_pattern_btn->labelsize(12);
    m_dec_pattern_btn->callback(cb_dec_pattern, this);

    m_inc_pattern_btn = new Fl_Button(x + 60, y + btn_y_offset + btn_h, 60, btn_h, "+");
    m_inc_pattern_btn->labelsize(12);
    m_inc_pattern_btn->callback(cb_inc_pattern, this);

    Fl_Light_Button* follow_btn = new Fl_Light_Button(x, y + btn_y_offset + 2 * btn_h, 120, btn_h, "Follow");
    follow_btn->labelsize(10);
    follow_btn->callback(cb_follow_playback, this);

    int content_y = y + btn_y_offset + 3 * btn_h;
    int content_h = h - (btn_y_offset + 3 * btn_h);
    int pattern_list_width = 120;

    m_pattern_scroll = new Fl_Scroll(x, content_y, pattern_list_width, content_h);
    m_pattern_scroll->type(Fl_Scroll::VERTICAL);
    m_pattern_list_container = new Fl_Group(x, content_y, pattern_list_width, 1000);
    m_pattern_list_container->end();
        m_pattern_scroll->add(m_pattern_list_container);
        m_pattern_scroll->end();
    
        m_main_scroll = new Fl_Scroll(x + pattern_list_width, content_y, w - pattern_list_width, content_h);
        m_main_scroll->type(0); // Hide both scrollbars
        m_tracker = new TrackerView(x + pattern_list_width, content_y, m_main_scroll->w(), m_main_scroll->h(), m_engine.pattern(), m_engine);
        m_main_scroll->add(m_tracker);
        m_main_scroll->end();
        
        resizable(m_main_scroll);
        end();
    
        update_pattern_list_browser();
    }
    
    void TrackerPanel::resize(int x, int y, int w, int h) {
        Fl_Group::resize(x, y, w, h);
        if (m_tracker && m_main_scroll) {
            m_tracker->recalculate_size();
        }
    }
void TrackerPanel::cb_detach(Fl_Widget*, void* data) {
    TrackerPanel* self = static_cast<TrackerPanel*>(data);
    Fl_Group* parent_grp = self->parent();
    if (self->m_detached_window) {
        self->m_detached_window->show();
    } else {
        self->m_detached_window = new DetachedWindow(400, 300, "Tracker", self, parent_grp);
        self->m_detached_window->show();
    }
    self->hide();
}


void TrackerPanel::cb_follow_playback(Fl_Widget* w, void* data) {
    TrackerPanel* self = static_cast<TrackerPanel*>(data);
    Fl_Light_Button* btn = static_cast<Fl_Light_Button*>(w);
    self->m_follow_playback = btn->value();
}

void TrackerPanel::update() {
    if (m_tracker) {
        if (m_follow_playback) {
            m_engine.m_edit_order_pos.store(m_engine.m_order_pos.load());
        }

        size_t current_edit_pos = m_engine.m_edit_order_pos.load();
        size_t current_order_size = m_engine.order_list().size();
        size_t current_pattern_count = m_engine.pattern_count();

        if ((int)current_edit_pos != m_selected_order_idx || 
            current_order_size != m_last_order_size ||
            current_pattern_count != m_last_pattern_count) {
            
            m_selected_order_idx = (int)current_edit_pos;
            update_pattern_list_browser();
        }

        if (m_engine.transport_state() != TransportState::Stopped) {
            m_tracker->ensure_cursor_visible();
        }
        m_tracker->redraw();
    }
}

void TrackerPanel::update_pattern_list_browser() {
    const auto& order = m_engine.order_list();
    m_last_order_size = order.size();
    m_last_pattern_count = m_engine.pattern_count();

    m_pattern_list_container->clear();
    m_pattern_list_container->begin();
    int row_h = 25;
    int start_y = m_pattern_list_container->y();
    int start_x = m_pattern_list_container->x();

    for (size_t i = 0; i < order.size(); ++i) {
        int cur_y = start_y + (int)(i * row_h);
        
        char pos_str[16];
        snprintf(pos_str, 16, "%02zu:", i);
        Fl_Button* b = new Fl_Button(start_x, cur_y, 30, row_h);
        b->copy_label(pos_str);
        b->box(m_selected_order_idx == (int)i ? FL_DOWN_BOX : FL_FLAT_BOX);
        if (m_selected_order_idx == (int)i) b->color(FL_SELECTION_COLOR);
        b->labelsize(12);
        b->user_data((void*)(uintptr_t)i);
        b->callback([](Fl_Widget* w, void* d){
            TrackerPanel* self = static_cast<TrackerPanel*>(d);
            if (!self) return;
            size_t idx = (size_t)(uintptr_t)w->user_data();
            auto& eng = self->m_engine;
            auto ord = eng.order_list();
            if (idx < ord.size()) {
                eng.m_edit_order_pos.store(idx);
                eng.set_active_pattern(ord[idx]);
                self->m_tracker->set_pattern(eng.pattern());
            }
        }, this);
        
        char pat_str[16];
        snprintf(pat_str, 16, "%02u", order[i]);
        Fl_Box* p = new Fl_Box(start_x + 30, cur_y, 30, row_h);
        p->copy_label(pat_str);
        p->labelsize(12);
        p->labelcolor(FL_YELLOW);
        
        // Pattern length input
        int pat_idx = order[i];
        size_t len = m_engine.pattern(pat_idx).row_count();
        char len_str[16];
        snprintf(len_str, 16, "%zu", len);
        Fl_Int_Input* len_inp = new Fl_Int_Input(start_x + 65, cur_y + 2, 40, 20);
        len_inp->value(len_str);
        len_inp->labelsize(10);
        len_inp->textsize(10);
        len_inp->when(FL_WHEN_ENTER_KEY_ALWAYS);
        len_inp->user_data((void*)(uintptr_t)pat_idx);
        len_inp->callback(cb_pattern_length, this);
    }
    m_pattern_list_container->end();
    m_pattern_list_container->size(m_pattern_list_container->w(), (int)(order.size() * row_h));
    m_pattern_scroll->redraw();
}

void TrackerPanel::grab_focus() {
    if (m_tracker) m_tracker->take_focus();
}

void TrackerPanel::cb_add_pattern(Fl_Widget*, void* data) {
    TrackerPanel* self = static_cast<TrackerPanel*>(data);
    self->m_engine.add_pattern_to_order();
    for (Fl_Window* win = Fl::first_window(); win; win = Fl::next_window(win)) {
        MainWindow* mw = dynamic_cast<MainWindow*>(win);
        if (mw) mw->request_update();
    }
}

void TrackerPanel::cb_remove_pattern(Fl_Widget*, void* data) {
    TrackerPanel* self = static_cast<TrackerPanel*>(data);
    const auto& order = self->m_engine.order_list();
    if (!order.empty()) {
        self->m_engine.remove_pattern_from_order(order.size() - 1);
        for (Fl_Window* win = Fl::first_window(); win; win = Fl::next_window(win)) {
            MainWindow* mw = dynamic_cast<MainWindow*>(win);
            if (mw) mw->request_update();
        }
    }
}

void TrackerPanel::cb_copy_pattern(Fl_Widget*, void* data) {
    TrackerPanel* self = static_cast<TrackerPanel*>(data);
    const auto& order = self->m_engine.order_list();
    if (!order.empty()) {
        self->m_engine.copy_pattern_in_order(order.size() - 1);
        for (Fl_Window* win = Fl::first_window(); win; win = Fl::next_window(win)) {
            MainWindow* mw = dynamic_cast<MainWindow*>(win);
            if (mw) mw->request_update();
        }
    }
}

void TrackerPanel::cb_inc_pattern(Fl_Widget*, void* data) {
    TrackerPanel* self = static_cast<TrackerPanel*>(data);
    size_t pos = self->m_engine.m_edit_order_pos.load();
    auto order = self->m_engine.order_list();
    if (pos < order.size()) {
        if (order[pos] < self->m_engine.pattern_count() - 1) {
             order[pos]++;
             self->m_engine.set_order(order);
             self->m_engine.set_active_pattern(order[pos]);
             self->m_tracker->set_pattern(self->m_engine.pattern());
             for (Fl_Window* win = Fl::first_window(); win; win = Fl::next_window(win)) {
                 MainWindow* mw = dynamic_cast<MainWindow*>(win);
                 if (mw) mw->request_update();
             }
        }
    }
}

void TrackerPanel::cb_dec_pattern(Fl_Widget*, void* data) {
    TrackerPanel* self = static_cast<TrackerPanel*>(data);
    size_t pos = self->m_engine.m_edit_order_pos.load();
    auto order = self->m_engine.order_list();
    if (pos < order.size()) {
        if (order[pos] > 0) {
            order[pos]--;
            self->m_engine.set_order(order);
            self->m_engine.set_active_pattern(order[pos]);
            self->m_tracker->set_pattern(self->m_engine.pattern());
            for (Fl_Window* win = Fl::first_window(); win; win = Fl::next_window(win)) {
                MainWindow* mw = dynamic_cast<MainWindow*>(win);
                if (mw) mw->request_update();
            }
        }
    }
}

void TrackerPanel::cb_pattern_length(Fl_Widget* w, void* data) {
    TrackerPanel* self = static_cast<TrackerPanel*>(data);
    size_t pat_idx = (size_t)(uintptr_t)w->user_data();
    Fl_Int_Input* inp = static_cast<Fl_Int_Input*>(w);
    int new_len = atoi(inp->value());
    printf("DEBUG: cb_pattern_length: pat_idx=%zu, value='%s', parsed=%d\n", pat_idx, inp->value(), new_len);
    if (new_len > 0 && new_len <= 512) {
        self->m_engine.resize_pattern(pat_idx, (size_t)new_len);
        if (self->m_tracker) {
            self->m_tracker->recalculate_size();
            self->m_tracker->redraw();
            if (self->m_main_scroll) self->m_main_scroll->redraw();
        }
        for (Fl_Window* win = Fl::first_window(); win; win = Fl::next_window(win)) {
            MainWindow* mw = dynamic_cast<MainWindow*>(win);
            if (mw) mw->request_update();
        }
    }
}

} // namespace disgrace_ns
