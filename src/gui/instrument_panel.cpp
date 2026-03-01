#include "instrument_panel.h"
#include "detached_window.h"
#include "main_window.h"
#include "../core/engine.h"
#include "../instrument/sample_instrument.h"
#include "../io/audio_file.h"
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Check_Button.H>
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
    
    // Sampler Editor
    m_sampler_editor = new Fl_Group(x + left_w, y, w - left_w, h);
    m_sampler_editor->begin();
    
    int middle_w = 150; // Shrunk middle plane
    int split_x = x + left_w + middle_w;

    // Sub-pane 1: Sample List (Middle Plane)
    m_sampler_list_grp = new Fl_Group(x + left_w, y, middle_w, h, "Samples");
    m_sampler_list_grp->box(FL_ENGRAVED_FRAME);
    m_sampler_list_grp->align(FL_ALIGN_TOP_LEFT);
    m_sampler_list_grp->begin();

    int rcur_y = margin;
    m_add_sample_btn = new Fl_Button(x + left_w + margin, y + rcur_y, middle_w - 2 * margin, 25, "+ Add");
    m_add_sample_btn->callback(cb_add_sample, this);
    
    rcur_y += 25 + margin;
    
    m_sample_scroll = new Fl_Scroll(x + left_w + margin, y + rcur_y, middle_w - 2 * margin, y + h - (y + rcur_y) - margin);
    m_sample_scroll->type(Fl_Scroll::VERTICAL);
    m_sample_container = new Fl_Group(x + left_w + margin, y + rcur_y, middle_w - 40, 1000);
    m_sample_container->end();
    m_sample_scroll->add(m_sample_container);
    m_sample_scroll->end();
    
    m_sampler_list_grp->end();

    // Sub-pane 2: Recording / Waveform (Right Plane)
    m_sampler_rec_grp = new Fl_Group(split_x, y, w - left_w - middle_w, h, "Waveform");
    m_sampler_rec_grp->box(FL_ENGRAVED_FRAME);
    m_sampler_rec_grp->align(FL_ALIGN_TOP_LEFT);
    m_sampler_rec_grp->begin();

    int rec_y = margin;
    m_rec_btn = new Fl_Button(split_x + margin, y + rec_y, 80, 25, "Record");
    m_rec_btn->callback(cb_record_sample, this);
    m_rec_btn->labelcolor(FL_RED);

    m_rec_input_ch = new Fl_Choice(split_x + margin + 85, y + rec_y, 120, 25);
    
    rec_y += 25 + margin;
    m_mono_btn = new Fl_Check_Button(split_x + margin, y + rec_y, 80, 25, "Mono");
    m_mono_btn->value(0); // Stereo by default
    m_mono_btn->callback(cb_mono_toggle, this);

    rec_y += 30 + margin;
    
    m_waveform_view = new WaveformView(split_x + margin, y + rec_y, w - split_x - 2 * margin, y + h - (y + rec_y) - margin);
    
    m_sampler_rec_grp->end();

    m_sampler_editor->end();
    m_sampler_editor->hide();

    m_right_panel->end();

    resizable(m_right_panel);

    end();

    update_instrument_list();
    update_editor();
    update_rec_inputs();
}

void InstrumentPanel::update_rec_inputs() {
    m_rec_input_ch->clear();
    bool mono = m_mono_btn->value();
    uint32_t num_ins = m_engine.m_num_ins;

    if (mono) {
        for (uint32_t i = 0; i < num_ins; ++i) {
            char buf[32];
            snprintf(buf, 32, "Channel %u", i + 1);
            m_rec_input_ch->add(strdup(buf));
        }
    } else {
        // Stereo pairs
        for (uint32_t i = 0; i < num_ins; i += 2) {
            char buf[32];
            if (i + 1 < num_ins) {
                snprintf(buf, 32, "Channels %u/%u", i + 1, i + 2);
            } else {
                snprintf(buf, 32, "Channel %u (L)", i + 1);
            }
            m_rec_input_ch->add(strdup(buf));
        }
    }
    m_rec_input_ch->value(0);
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

        Fl_Button* sel_btn = new Fl_Button(cur_x, cur_y, label_w, row_h, strdup((std::to_string(i+1) + ":").c_str()));
        sel_btn->box(FL_NO_BOX);
        if ((int)i == m_selected_instrument) sel_btn->labelcolor(FL_YELLOW);
        else sel_btn->labelcolor(FL_FOREGROUND_COLOR);
        sel_btn->callback(cb_inst_select, new std::pair<InstrumentPanel*, size_t>(this, i));
        cur_x += label_w + 5;

        Fl_Input* name_in = new Fl_Input(cur_x, cur_y + 5, input_w, 25);
        name_in->value(inst.name().c_str());
        name_in->maximum_size(64);
        name_in->callback(cb_inst_name, new std::pair<InstrumentPanel*, size_t>(this, i));
        name_in->when(FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);
        cur_x += input_w + 5;

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
        int btn_w = 20;
        
        for (size_t i = 0; i < sampler->sample_count(); ++i) {
            int cur_y = start_y + (int)(i * row_h);
            int cur_x = start_x;
            const auto& entry = sampler->get_sample(i);
            
            Fl_Button* sel = new Fl_Button(cur_x, cur_y, 25, 25, strdup((std::to_string(i+1) + ":").c_str()));
            sel->box(FL_NO_BOX);
            if ((int)i == m_selected_sample) sel->labelcolor(FL_YELLOW);
            sel->callback(cb_sample_select, new std::pair<InstrumentPanel*, size_t>(this, i));
            cur_x += 30;

            Fl_Input* name_in = new Fl_Input(cur_x, cur_y + 5, 60, 25);
            name_in->value(entry.name.c_str());
            name_in->labelsize(10);
            name_in->callback(cb_sample_name, new std::pair<InstrumentPanel*, size_t>(this, i));
            name_in->when(FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);
            cur_x += 65;
            
            Fl_Button* load = new Fl_Button(cur_x, cur_y + 5, btn_w, 25, "L");
            load->labelsize(10);
            load->callback(cb_load_sample, new std::pair<InstrumentPanel*, size_t>(this, i));
            cur_x += btn_w + 2;

            Fl_Button* up = new Fl_Button(cur_x, cur_y + 5, btn_w, 25, "U");
            up->labelsize(10);
            up->callback(cb_move_sample_up, new std::pair<InstrumentPanel*, size_t>(this, i));
            cur_x += btn_w + 2;
            
            Fl_Button* down = new Fl_Button(cur_x, cur_y + 5, btn_w, 25, "D");
            down->labelsize(10);
            down->callback(cb_move_sample_down, new std::pair<InstrumentPanel*, size_t>(this, i));
            cur_x += btn_w + 2;
            
            Fl_Button* rem = new Fl_Button(cur_x, cur_y + 5, btn_w, 25, "X");
            rem->labelsize(10);
            rem->callback(cb_remove_sample, new std::pair<InstrumentPanel*, size_t>(this, i));
        }
        
        m_sample_container->end();
        m_sample_container->size(m_sample_container->w(), (int)(sampler->sample_count() * row_h + 20));
        m_sample_scroll->redraw();

        // Update waveform view
        if (m_selected_sample >= 0 && m_selected_sample < (int)sampler->sample_count()) {
            m_waveform_view->set_sample(sampler->get_sample(m_selected_sample).data);
        } else {
            m_waveform_view->set_sample(nullptr);
        }
        
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
}

void InstrumentPanel::cb_inst_select(Fl_Widget*, void* data) {
    auto* pair = static_cast<std::pair<InstrumentPanel*, size_t>*>(data);
    pair->first->m_selected_instrument = (int)pair->second;
    pair->first->m_selected_sample = -1;
    pair->first->update_instrument_list();
    pair->first->update_editor();
    delete pair;
}

void InstrumentPanel::cb_sample_select(Fl_Widget*, void* data) {
    auto* pair = static_cast<std::pair<InstrumentPanel*, size_t>*>(data);
    pair->first->m_selected_sample = (int)pair->second;
    pair->first->update_editor();
    delete pair;
}

void InstrumentPanel::cb_load(Fl_Widget*, void*) {}
void InstrumentPanel::cb_save(Fl_Widget*, void*) {}

void InstrumentPanel::cb_delete(Fl_Widget*, void* data) {
    auto* self = static_cast<InstrumentPanel*>(data);
    if (self->m_selected_instrument >= 0) {
        if (fl_ask("Delete selected instrument?")) {
            self->m_engine.remove_instrument(self->m_selected_instrument);
            self->m_selected_instrument = -1;
            self->m_selected_sample = -1;
            self->update_instrument_list();
            self->update_editor();
        }
    }
}

void InstrumentPanel::cb_inst_name(Fl_Widget* w, void* data) {
    auto* pair = static_cast<std::pair<InstrumentPanel*, size_t>*>(data);
    Fl_Input* in = static_cast<Fl_Input*>(w);
    pair->first->m_engine.instrument(pair->second).set_name(in->value());
}

void InstrumentPanel::cb_inst_type(Fl_Widget* w, void* data) {
    auto* pair = static_cast<std::pair<InstrumentPanel*, size_t>*>(data);
    Fl_Choice* ch = static_cast<Fl_Choice*>(w);
    pair->first->m_engine.set_instrument_type(pair->second, (InstrumentType)ch->value());
    pair->first->m_selected_instrument = (int)pair->second;
    pair->first->m_selected_sample = -1;
    pair->first->update_instrument_list();
    pair->first->update_editor();
}

void InstrumentPanel::cb_add_sample(Fl_Widget*, void* data) {
    auto* self = static_cast<InstrumentPanel*>(data);
    if (self->m_selected_instrument < 0) return;
    auto& inst = self->m_engine.instrument(self->m_selected_instrument);
    if (inst.type() == InstrumentType::Sampler) {
        static_cast<SampleInstrument*>(&inst)->add_sample("New Sample", nullptr);
        self->update_editor();
    }
}

void InstrumentPanel::cb_load_sample(Fl_Widget*, void* data) {
    auto* pair = static_cast<std::pair<InstrumentPanel*, size_t>*>(data);
    InstrumentPanel* self = pair->first;
    size_t sample_idx = pair->second;

    Fl_Native_File_Chooser fnfc;
    fnfc.title("Load Sample");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fnfc.filter("Audio Files\t*.{wav,flac,mp3}\n");

    if (fnfc.show() == 0) {
        auto& inst = self->m_engine.instrument(self->m_selected_instrument);
        if (inst.type() == InstrumentType::Sampler) {
            auto* sampler = static_cast<SampleInstrument*>(&inst);
            auto data_ptr = std::make_shared<SampleData>();
            uint32_t sr = 0;
            if (AudioFile::load_audio(fnfc.filename(), data_ptr->left, data_ptr->right, sr)) {
                data_ptr->sample_rate = (int)sr;
                std::string path = fnfc.filename();
                size_t last_slash = path.find_last_of("/\\");
                std::string name = (last_slash == std::string::npos) ? path : path.substr(last_slash + 1);
                
                sampler->set_sample_name(sample_idx, name);
                sampler->get_sample(sample_idx).data = data_ptr;
                self->m_selected_sample = (int)sample_idx;
                self->update_editor();
            }
        }
    }
    delete pair;
}

void InstrumentPanel::cb_remove_sample(Fl_Widget*, void* data) {
    auto* pair = static_cast<std::pair<InstrumentPanel*, size_t>*>(data);
    SampleInstrument* sampler = static_cast<SampleInstrument*>(&pair->first->m_engine.instrument(pair->first->m_selected_instrument));
    sampler->remove_sample(pair->second);
    pair->first->m_selected_sample = -1;
    pair->first->update_editor();
    delete pair;
}

void InstrumentPanel::cb_move_sample_up(Fl_Widget*, void* data) {
    auto* pair = static_cast<std::pair<InstrumentPanel*, size_t>*>(data);
    if (pair->second > 0) {
        SampleInstrument* sampler = static_cast<SampleInstrument*>(&pair->first->m_engine.instrument(pair->first->m_selected_instrument));
        sampler->move_sample(pair->second, pair->second - 1);
        pair->first->m_selected_sample = (int)pair->second - 1;
        pair->first->update_editor();
    }
    delete pair;
}

void InstrumentPanel::cb_move_sample_down(Fl_Widget*, void* data) {
    auto* pair = static_cast<std::pair<InstrumentPanel*, size_t>*>(data);
    SampleInstrument* sampler = static_cast<SampleInstrument*>(&pair->first->m_engine.instrument(pair->first->m_selected_instrument));
    if (pair->second < sampler->sample_count() - 1) {
        sampler->move_sample(pair->second, pair->second + 1);
        pair->first->m_selected_sample = (int)pair->second + 1;
        pair->first->update_editor();
    }
    delete pair;
}

void InstrumentPanel::cb_save_sample(Fl_Widget*, void*) {}

void InstrumentPanel::cb_sample_name(Fl_Widget* w, void* data) {
    auto* pair = static_cast<std::pair<InstrumentPanel*, size_t>*>(data);
    Fl_Input* in = static_cast<Fl_Input*>(w);
    SampleInstrument* sampler = static_cast<SampleInstrument*>(&pair->first->m_engine.instrument(pair->first->m_selected_instrument));
    sampler->set_sample_name(pair->second, in->value());
}

void InstrumentPanel::cb_record_sample(Fl_Widget*, void*) {}

void InstrumentPanel::cb_mono_toggle(Fl_Widget*, void* data) {
    auto* self = static_cast<InstrumentPanel*>(data);
    self->update_rec_inputs();
}

void InstrumentPanel::cb_detach(Fl_Widget*, void* data) {
    InstrumentPanel* self = static_cast<InstrumentPanel*>(data);
    Fl_Group* parent_grp = self->parent();
    if (self->m_detached_window) self->m_detached_window->show();
    else {
        self->m_detached_window = new DetachedWindow(800, 600, "Instruments", self, parent_grp);
        self->m_detached_window->show();
    }
    self->hide();
}

} // namespace disgrace_ns
