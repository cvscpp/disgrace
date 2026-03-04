#include "tracker_panel.h"
#include "tracker_view.h"
#include "detached_window.h"
#include "../core/engine.h"
#include <FL/Fl_Box.H>
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

    int content_y = y + btn_y_offset + btn_h;
    int content_h = h - btn_h;
    int pattern_list_width = 120;

    m_pattern_scroll = new Fl_Scroll(x, content_y, pattern_list_width, content_h);
    m_pattern_scroll->type(Fl_Scroll::VERTICAL);
    m_pattern_list_container = new Fl_Group(x, content_y, pattern_list_width, 1000);
    m_pattern_list_container->end();
        m_pattern_scroll->add(m_pattern_list_container);
        m_pattern_scroll->end();
    
        m_main_scroll = new Fl_Scroll(x + pattern_list_width, content_y, w - pattern_list_width, content_h);
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


void TrackerPanel::update() {
    if (m_tracker) m_tracker->redraw();
}

void TrackerPanel::update_pattern_list_browser() {
    m_pattern_list_container->clear();
    m_pattern_list_container->begin();
    const auto& order = m_engine.order_list();
    int row_h = 25;
    int start_y = m_pattern_list_container->y();
    int start_x = m_pattern_list_container->x();

    for (size_t i = 0; i < order.size(); ++i) {
        int cur_y = start_y + (int)(i * row_h);
        
        char pos_str[16];
        snprintf(pos_str, 16, "%02zu:", i);
        Fl_Box* b = new Fl_Box(start_x, cur_y, 30, row_h, strdup(pos_str));
        b->labelsize(12);
        
        char pat_str[16];
        snprintf(pat_str, 16, "%02u", order[i]);
        Fl_Box* p = new Fl_Box(start_x + 30, cur_y, 30, row_h, strdup(pat_str));
        p->labelsize(12);
        p->labelcolor(FL_YELLOW);
        
        Fl_Button* dec = new Fl_Button(start_x + 65, cur_y + 2, 20, 20, "-");
        dec->labelsize(10);
        dec->callback(cb_dec_pattern, new std::pair<TrackerPanel*, size_t>(this, i));
        
        Fl_Button* inc = new Fl_Button(start_x + 90, cur_y + 2, 20, 20, "+");
        inc->labelsize(10);
        inc->callback(cb_inc_pattern, new std::pair<TrackerPanel*, size_t>(this, i));
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
    self->update_pattern_list_browser();
}

void TrackerPanel::cb_remove_pattern(Fl_Widget*, void* data) {
    TrackerPanel* self = static_cast<TrackerPanel*>(data);
    const auto& order = self->m_engine.order_list();
    if (!order.empty()) {
        self->m_engine.remove_pattern_from_order(order.size() - 1);
        self->update_pattern_list_browser();
    }
}

void TrackerPanel::cb_copy_pattern(Fl_Widget*, void* data) {
    TrackerPanel* self = static_cast<TrackerPanel*>(data);
    const auto& order = self->m_engine.order_list();
    if (!order.empty()) {
        self->m_engine.copy_pattern_in_order(order.size() - 1);
        self->update_pattern_list_browser();
    }
}

void TrackerPanel::cb_inc_pattern(Fl_Widget*, void* data) {
    auto* pair = static_cast<std::pair<TrackerPanel*, size_t>*>(data);
    TrackerPanel* self = pair->first;
    size_t pos = pair->second;
    auto order = self->m_engine.order_list();
    if (pos < order.size()) {
        order[pos]++;
        if (order[pos] >= self->m_engine.pattern_count()) {
             order[pos] = self->m_engine.pattern_count() - 1;
        }
        self->m_engine.set_order(order);
        self->update_pattern_list_browser();
    }
    delete pair;
}

void TrackerPanel::cb_dec_pattern(Fl_Widget*, void* data) {
    auto* pair = static_cast<std::pair<TrackerPanel*, size_t>*>(data);
    TrackerPanel* self = pair->first;
    size_t pos = pair->second;
    auto order = self->m_engine.order_list();
    if (pos < order.size()) {
        if (order[pos] > 0) {
            order[pos]--;
            self->m_engine.set_order(order);
            self->update_pattern_list_browser();
        }
    }
    delete pair;
}

} // namespace disgrace_ns
