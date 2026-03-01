#include "instrument_panel.h"
#include "detached_window.h"
#include "main_window.h"
#include "../core/engine.h"
#include "../instrument/sample_instrument.h"
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_ask.H>

namespace disgrace_ns {

InstrumentPanel::InstrumentPanel(int x, int y, int w, int h, Engine& engine)
    : Fl_Group(x, y, w, h), m_engine(engine) {
    
    begin();

    int left_w = 350;
    int margin = 10;

    m_detach_btn = new Fl_Button(x + w - 30, y, 30, 20, "[]");
    m_detach_btn->callback(cb_detach, this);

    // Left Panel
    m_left_panel = new Fl_Group(x, y, left_w, h, "Instruments");
    m_left_panel->box(FL_ENGRAVED_FRAME);
    m_left_panel->align(FL_ALIGN_TOP_LEFT);
    m_left_panel->begin();

    int cur_y = margin;
    int btn_w = (left_w - 5 * margin) / 4;

    m_new_btn = new Fl_Button(x + margin, y + cur_y, btn_w, 25, "New");
    m_new_btn->callback(cb_new, this);

    m_load_btn = new Fl_Button(x + 2 * margin + btn_w, y + cur_y, btn_w, 25, "Load");
    m_load_btn->callback(cb_load, this);

    m_save_btn = new Fl_Button(x + 3 * margin + 2 * btn_w, y + cur_y, btn_w, 25, "Save");
    m_save_btn->callback(cb_save, this);

    m_delete_btn = new Fl_Button(x + 4 * margin + 3 * btn_w, y + cur_y, btn_w, 25, "Del");
    m_delete_btn->callback(cb_delete, this);

    cur_y += 25 + margin;

    m_inst_scroll = new Fl_Scroll(x + margin, y + cur_y, left_w - 2 * margin, 200);
    m_inst_scroll->type(Fl_Scroll::VERTICAL);
    m_inst_container = new Fl_Group(x + margin, y + cur_y, left_w - 40, 1000);
    m_inst_container->end();
    m_inst_scroll->add(m_inst_container);
    m_inst_scroll->end();

    cur_y += 200 + margin;

    m_file_browser = new Fl_File_Browser(x + margin, y + cur_y, left_w - 2 * margin, y + h - (y + cur_y) - margin);
    m_file_browser->load(".");

    m_left_panel->end();

    // Right Panel (Editor)
    m_right_panel = new Fl_Group(x + left_w, y, w - left_w, h, "Editor");
    m_right_panel->box(FL_ENGRAVED_FRAME);
    m_right_panel->align(FL_ALIGN_TOP_LEFT);
    m_right_panel->begin();
    
    // Sampler Editor (Initially hidden)
    m_sampler_editor = new Fl_Group(x + left_w + margin, y + margin, w - left_w - 2 * margin, h - 2 * margin);
    m_sampler_editor->begin();
    
    int rcur_y = margin;
    m_add_sample_btn = new Fl_Button(x + left_w + margin, y + rcur_y, 100, 25, "+ Add Sample");
    m_add_sample_btn->callback(cb_add_sample, this);
    
    rcur_y += 25 + margin;
    
    m_sample_scroll = new Fl_Scroll(x + left_w + margin, y + rcur_y, w - left_w - 2 * margin, y + h - (y + rcur_y) - margin);
    m_sample_scroll->type(Fl_Scroll::VERTICAL);
    m_sample_container = new Fl_Group(x + left_w + margin, y + rcur_y, w - left_w - 40, 1000);
    m_sample_container->end();
    m_sample_scroll->add(m_sample_container);
    m_sample_scroll->end();
    
    m_sampler_editor->end();
    m_sampler_editor->hide();

    m_right_panel->end();

    resizable(m_right_panel);

    end();

    update_instrument_list();
    update_editor();
}

void InstrumentPanel::update_instrument_list() {
    m_inst_container->clear();
    m_inst_container->begin();

    int row_h = 35;
    int start_y = m_inst_container->y(); 
    int start_x = m_inst_container->x();
    int label_w = 30;
    int input_w = 120;
    int choice_w = 100;

    size_t num_insts = m_engine.instrument_count();

    for (size_t i = 0; i < num_insts; ++i) {
        int cur_y = start_y + (int)(i * row_h);
        int cur_x = start_x;
        auto& inst = m_engine.instrument(i);

        // Selection / Index
        Fl_Button* sel_btn = new Fl_Button(cur_x, cur_y, label_w, row_h, strdup((std::to_string(i+1) + ":").c_str()));
        sel_btn->box(FL_NO_BOX);
        if ((int)i == m_selected_instrument) sel_btn->labelcolor(FL_YELLOW);
        else sel_btn->labelcolor(FL_FOREGROUND_COLOR);
        sel_btn->callback(cb_inst_select, new std::pair<InstrumentPanel*, size_t>(this, i));
        cur_x += label_w + 5;

        // Name
        Fl_Input* name_in = new Fl_Input(cur_x, cur_y + 5, input_w, 25);
        name_in->value(inst.name().c_str());
        name_in->maximum_size(64);
        name_in->callback(cb_inst_name, new std::pair<InstrumentPanel*, size_t>(this, i));
        name_in->when(FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);
        cur_x += input_w + 5;

        // Type
        Fl_Choice* type_ch = new Fl_Choice(cur_x, cur_y + 5, choice_w, 25);
        type_ch->add("None");
        type_ch->add("Sampler");
        type_ch->add("SoundFont");
        type_ch->add("Plugin");
        type_ch->add("MIDI");
        type_ch->value((int)inst.type());
        type_ch->callback(cb_inst_type, new std::pair<InstrumentPanel*, size_t>(this, i));

        cur_y += row_h;
    }

    m_inst_container->end();
    m_inst_container->size(m_inst_container->w(), (int)(num_insts * row_h + 20));
    m_inst_scroll->redraw();
}

void InstrumentPanel::update_editor() {
    if (m_selected_instrument < 0 || m_selected_instrument >= (int)m_engine.instrument_count()) {
        m_sampler_editor->hide();
        m_right_panel->redraw();
        return;
    }

    auto& inst = m_engine.instrument(m_selected_instrument);
    if (inst.type() == InstrumentType::Sampler) {
        m_sampler_editor->show();
        
        m_sample_container->clear();
        m_sample_container->begin();
        
        SampleInstrument* sampler = static_cast<SampleInstrument*>(&inst);
        int row_h = 35;
        int start_y = m_sample_container->y();
        int start_x = m_sample_container->x();
        int btn_w = 40;
        
        for (size_t i = 0; i < sampler->sample_count(); ++i) {
            int cur_y = start_y + (int)(i * row_h);
            int cur_x = start_x;
            const auto& entry = sampler->get_sample(i);
            
            Fl_Box* name_box = new Fl_Box(cur_x, cur_y, 150, 25, strdup(entry.name.c_str()));
            name_box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            cur_x += 155;
            
            Fl_Button* up = new Fl_Button(cur_x, cur_y, btn_w, 25, "Up");
            up->labelsize(10);
            up->callback(cb_move_sample_up, new std::pair<InstrumentPanel*, size_t>(this, i));
            cur_x += btn_w + 2;
            
            Fl_Button* down = new Fl_Button(cur_x, cur_y, btn_w, 25, "Dn");
            down->labelsize(10);
            down->callback(cb_move_sample_down, new std::pair<InstrumentPanel*, size_t>(this, i));
            cur_x += btn_w + 5;
            
            Fl_Button* save = new Fl_Button(cur_x, cur_y, btn_w, 25, "Save");
            save->labelsize(10);
            save->callback(cb_save_sample, new std::pair<InstrumentPanel*, size_t>(this, i));
            cur_x += btn_w + 5;
            
            Fl_Button* rem = new Fl_Button(cur_x, cur_y, btn_w, 25, "X");
            rem->callback(cb_remove_sample, new std::pair<InstrumentPanel*, size_t>(this, i));
        }
        
        m_sample_container->end();
        m_sample_container->size(m_sample_container->w(), (int)(sampler->sample_count() * row_h + 20));
        m_sample_scroll->redraw();
        
    } else {
        m_sampler_editor->hide();
    }
    m_right_panel->redraw();
}

void InstrumentPanel::cb_new(Fl_Widget*, void* data) {
    auto* self = static_cast<InstrumentPanel*>(data);
    self->m_engine.add_instrument();
    self->m_selected_instrument = (int)self->m_engine.instrument_count() - 1;
    self->update_instrument_list();
    self->update_editor();
    for (Fl_Window* win = Fl::first_window(); win; win = Fl::next_window(win)) {
        MainWindow* mw = dynamic_cast<MainWindow*>(win);
        if (mw) mw->update_all_uis();
    }
}

void InstrumentPanel::cb_inst_select(Fl_Widget* w, void* data) {
    auto* pair = static_cast<std::pair<InstrumentPanel*, size_t>*>(data);
    pair->first->m_selected_instrument = (int)pair->second;
    pair->first->update_instrument_list();
    pair->first->update_editor();
    delete pair;
}

void InstrumentPanel::cb_load(Fl_Widget*, void* data) {}
void InstrumentPanel::cb_save(Fl_Widget*, void* data) {}

void InstrumentPanel::cb_delete(Fl_Widget*, void* data) {
    auto* self = static_cast<InstrumentPanel*>(data);
    if (self->m_selected_instrument >= 0) {
        if (fl_ask("Delete selected instrument?")) {
            self->m_engine.remove_instrument(self->m_selected_instrument);
            self->m_selected_instrument = -1;
            self->update_instrument_list();
            self->update_editor();
            for (Fl_Window* win = Fl::first_window(); win; win = Fl::next_window(win)) {
                MainWindow* mw = dynamic_cast<MainWindow*>(win);
                if (mw) mw->update_all_uis();
            }
        }
    }
}

void InstrumentPanel::cb_inst_name(Fl_Widget* w, void* data) {
    auto* pair = static_cast<std::pair<InstrumentPanel*, size_t>*>(data);
    Fl_Input* in = static_cast<Fl_Input*>(w);
    pair->first->m_engine.instrument(pair->second).set_name(in->value());
    pair->first->m_selected_instrument = (int)pair->second;
    pair->first->update_instrument_list();
    pair->first->update_editor();
}

void InstrumentPanel::cb_inst_type(Fl_Widget* w, void* data) {
    auto* pair = static_cast<std::pair<InstrumentPanel*, size_t>*>(data);
    Fl_Choice* ch = static_cast<Fl_Choice*>(w);
    InstrumentType new_type = (InstrumentType)ch->value();
    
    pair->first->m_engine.set_instrument_type(pair->second, new_type);
    pair->first->m_selected_instrument = (int)pair->second;
    pair->first->update_instrument_list();
    pair->first->update_editor();
}

void InstrumentPanel::cb_add_sample(Fl_Widget*, void* data) {
    auto* self = static_cast<InstrumentPanel*>(data);
    if (self->m_selected_instrument < 0) return;
    auto& inst = self->m_engine.instrument(self->m_selected_instrument);
    if (inst.type() == InstrumentType::Sampler) {
        SampleInstrument* sampler = static_cast<SampleInstrument*>(&inst);
        sampler->add_sample("New Sample", nullptr);
        self->update_editor();
    }
}

void InstrumentPanel::cb_remove_sample(Fl_Widget*, void* data) {
    auto* pair = static_cast<std::pair<InstrumentPanel*, size_t>*>(data);
    SampleInstrument* sampler = static_cast<SampleInstrument*>(&pair->first->m_engine.instrument(pair->first->m_selected_instrument));
    sampler->remove_sample(pair->second);
    pair->first->update_editor();
    delete pair;
}

void InstrumentPanel::cb_move_sample_up(Fl_Widget*, void* data) {
    auto* pair = static_cast<std::pair<InstrumentPanel*, size_t>*>(data);
    if (pair->second > 0) {
        SampleInstrument* sampler = static_cast<SampleInstrument*>(&pair->first->m_engine.instrument(pair->first->m_selected_instrument));
        sampler->move_sample(pair->second, pair->second - 1);
        pair->first->update_editor();
    }
    delete pair;
}

void InstrumentPanel::cb_move_sample_down(Fl_Widget*, void* data) {
    auto* pair = static_cast<std::pair<InstrumentPanel*, size_t>*>(data);
    SampleInstrument* sampler = static_cast<SampleInstrument*>(&pair->first->m_engine.instrument(pair->first->m_selected_instrument));
    if (pair->second < sampler->sample_count() - 1) {
        sampler->move_sample(pair->second, pair->second + 1);
        pair->first->update_editor();
    }
    delete pair;
}

void InstrumentPanel::cb_save_sample(Fl_Widget*, void* data) {}

void InstrumentPanel::cb_detach(Fl_Widget*, void* data) {
    InstrumentPanel* self = static_cast<InstrumentPanel*>(data);
    Fl_Group* parent_grp = self->parent();
    if (self->m_detached_window) {
        self->m_detached_window->show();
    } else {
        self->m_detached_window = new DetachedWindow(800, 600, "Instruments", self, parent_grp);
        self->m_detached_window->show();
    }
    self->hide();
}

} // namespace disgrace_ns
