#include "mixer_panel.h"
#include "detached_window.h"
#include "../core/engine.h"
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Value_Input.H>
#include <FL/filename.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Native_File_Chooser.H>
#include "../dsp/gain.h"
#include "../dsp/delay.h"
#include <fstream>
#include <sstream>

namespace disgrace_ns {

MixerPanel::MixerPanel(int x, int y, int w, int h, Engine& engine)
    : Fl_Group(x, y, w, h), m_engine(engine) {
    
    begin();

    m_tile = new Fl_Tile(x, y, w, h);
    m_tile->begin();

    // Upper Pane: Tracks
    m_upper_pane = new Fl_Group(x, y, w, h / 2);
    m_upper_pane->box(FL_ENGRAVED_FRAME);
    m_upper_pane->begin();

    m_detach_btn = new Fl_Button(x + w - 30, y, 30, 20, "[]");
    m_detach_btn->callback(cb_detach, this);

    // Use a scroll area for tracks
    Fl_Scroll* scroll = new Fl_Scroll(x, y + 25, w, h / 2 - 25);
    scroll->type(Fl_Scroll::HORIZONTAL);

    // This group will hold the tracks
    m_track_group = new Fl_Group(x, y + 25, w, h / 2 - 25);
    m_track_group->end();
    scroll->add(m_track_group);
    scroll->end();
    m_upper_pane->end();

    // Lower Pane: Effects
    m_lower_pane = new Fl_Group(x, y + h / 2, w, h / 2);
    m_lower_pane->box(FL_ENGRAVED_FRAME);
    m_lower_pane->begin();
    
    int margin = 5;
    int cur_x = x + margin;
    int cur_y = y + h / 2 + margin;
    int std_h = m_engine.m_gui_button_height;

    // Available Effects Browser
    new Fl_Box(cur_x, cur_y, 120, 20, "Available Effects");
    m_avail_fx_browser = new Fl_Browser(cur_x, cur_y + 20, 120, h / 2 - 60);
    m_avail_fx_browser->type(FL_HOLD_BROWSER);
    m_avail_fx_browser->add("Gain");
    m_avail_fx_browser->add("Delay");
    
    Fl_Button* add_fx_btn = new Fl_Button(cur_x, y + h - 35, 120, std_h, "Add Effect");
    add_fx_btn->callback(cb_add_fx, this);

    cur_x += 120 + margin;

    // Selected Track Effect Chain
    new Fl_Box(cur_x, cur_y, 240, 20, "Effect Chain");
    m_fx_chain_group = new Fl_Group(cur_x, cur_y + 20, 240, h / 2 - 60);
    m_fx_chain_group->box(FL_DOWN_FRAME);
    m_fx_chain_group->end();

    m_load_chain_btn = new Fl_Button(cur_x, y + h - 35, 115, std_h, "Load Chain");
    m_load_chain_btn->callback(cb_load_chain, this);
    m_save_chain_btn = new Fl_Button(cur_x + 125, y + h - 35, 115, std_h, "Save Chain");
    m_save_chain_btn->callback(cb_save_chain, this);

    cur_x += 240 + margin;

    // Effect Parameters
    new Fl_Box(cur_x, cur_y, w - (cur_x - x) - margin, 20, "Effect Controls");
    m_fx_params_group = new Fl_Group(cur_x, cur_y + 20, w - (cur_x - x) - margin, h / 2 - 30);
    m_fx_params_group->box(FL_ENGRAVED_FRAME);
    m_fx_params_group->end();

    m_lower_pane->end();

    m_tile->end();

    // Master section
    m_upper_pane->begin();
    m_master_gain = new Fl_Value_Slider(x + w - 250, y + h/2 - 35, 150, 25, "Master");
    m_master_gain->type(FL_HOR_NICE_SLIDER);
    m_master_gain->range(0.0, 2.0);
    m_master_gain->value(1.0);
    m_master_gain->callback(cb_master_gain, this);

    m_master_meter_l = new VUMeter(x + w - 45, y + h/2 - 35, 15, 25);
    m_master_meter_r = new VUMeter(x + w - 25, y + h/2 - 35, 15, 25);

    Fl_Button* master_sel = new Fl_Button(x + w - 330, y + h/2 - 35, 75, 25, "SEL");
    master_sel->callback(cb_track_select, new ::std::pair<MixerPanel*,int>(this, -1));
    if (m_selected_track == -1) master_sel->color(FL_YELLOW);

    m_upper_pane->end();

    m_lower_pane->begin();
    m_spectral_view = new SpectralView(x + w - 200, y + h - 140, 190, 100, m_engine);
    m_lower_pane->end();

    end();

    update_mixer_ui();
    update_effect_editor();
}

void MixerPanel::update_mixer_ui() {
    for (int i = 0; i < m_track_group->children(); ++i) {
        void* d = m_track_group->child(i)->user_data();
        if (d) delete static_cast<std::pair<MixerPanel*, int>*>(d);
    }
    m_track_group->clear();
    m_track_group->begin();
    m_track_meters.clear();
    m_bus_meters.clear();

    size_t num_tracks = m_engine.track_count();
    size_t num_buses = m_engine.bus_count();
    
    int x_offset = 0;
    int std_h = m_engine.m_gui_button_height;
    int font_sz = m_engine.m_gui_font_size;

    for (size_t i = 0; i < num_tracks; ++i)
    {
      int cur_y = 5;
      Fl_Box* b = new Fl_Box(20 + x_offset, cur_y, 80, std_h, strdup(("Track " + ::std::to_string(i+1)).c_str()));
      b->labelsize(font_sz);
      cur_y += std_h + 5;
      
      Fl_Slider* vol = new Fl_Slider(20 + x_offset, cur_y, 20, 150);
      vol->type(FL_VERTICAL);
      vol->bounds(1, 0); 
      vol->value(m_engine.track(i).volume());
      vol->callback(cb_track_volume, new ::std::pair<MixerPanel*,int>(this, (int)i));

      VUMeter* meter_l = new VUMeter(45 + x_offset, cur_y, 8, 150);
      VUMeter* meter_r = new VUMeter(55 + x_offset, cur_y, 8, 150);
      m_track_meters.push_back({meter_l, meter_r});
      cur_y += 150 + 5;

      Fl_Slider* pan_slider = new Fl_Slider(20 + x_offset, cur_y, 75, 15);
      pan_slider->type(FL_HOR_SLIDER);
      pan_slider->range(-1.0, 1.0);
      pan_slider->value(m_engine.track(i).get_pan());
      pan_slider->callback(cb_track_pan, new ::std::pair<MixerPanel*,int>(this, (int)i));
      cur_y += 15 + 5;

      Fl_Check_Button* mute = new Fl_Check_Button(20 + x_offset, cur_y, 35, std_h, "M");
      mute->labelsize(font_sz);
      mute->value(m_engine.track(i).muted());
      mute->callback(cb_track_mute, new ::std::pair<MixerPanel*,int>(this, (int)i));

      Fl_Check_Button* solo = new Fl_Check_Button(60 + x_offset, cur_y, 35, std_h, "S");
      solo->labelsize(font_sz);
      solo->value(m_engine.track(i).solo());
      solo->callback(cb_track_solo, new ::std::pair<MixerPanel*,int>(this, (int)i));
      cur_y += std_h + 5;

      Fl_Button* sel = new Fl_Button(20 + x_offset, cur_y, 75, std_h, "SEL");
      sel->labelsize(font_sz);
      if ((int)i == m_selected_track) sel->color(FL_YELLOW);
      sel->callback(cb_track_select, new ::std::pair<MixerPanel*,int>(this, (int)i));
      cur_y += std_h + 5;

      // Audio Input for MIDI instruments
      if (m_engine.track(i).instrument() && m_engine.track(i).instrument()->type() == InstrumentType::Midi) {
        Fl_Choice* input_choice = new Fl_Choice(20 + x_offset, cur_y, 75, std_h);
        input_choice->labelsize(font_sz - 2);
        input_choice->add("None");
        uint32_t num_ins = m_engine.m_num_ins;
        for (uint32_t j = 0; j < num_ins; ++j) {
            char name[32];
            snprintf(name, sizeof(name), "In %u", j + 1);
            input_choice->add(strdup(name));
        }
        for (uint32_t j = 0; j + 1 < num_ins; j += 2) {
            char name[32];
            snprintf(name, sizeof(name), "In %u+%u", j + 1, j + 2);
            input_choice->add(strdup(name));
        }

        int cl, cr;
        m_engine.track(i).get_audio_input(cl, cr);
        if (cl == -1) input_choice->value(0);
        else if (cr == -1) input_choice->value(cl + 1);
        else if (cr == cl + 1) input_choice->value((int)(num_ins + 1 + cl / 2));
        
        input_choice->callback(cb_track_input, new ::std::pair<MixerPanel*,int>(this, (int)i));
        cur_y += std_h + 5;

        Fl_Value_Input* delay_input = new Fl_Value_Input(20 + x_offset + 30, cur_y, 45, std_h, "Dly");
        delay_input->labelsize(font_sz - 2);
        delay_input->value(m_engine.track(i).input_delay());
        delay_input->callback(cb_track_delay, new ::std::pair<MixerPanel*,int>(this, (int)i));
      }

      x_offset += 100;
    }

    if (num_buses > 0) {
      Fl_Box* separator = new Fl_Box(20 + x_offset - 5, 0, 5, 350);
      separator->box(FL_THIN_DOWN_BOX);
      x_offset += 10;

      for (size_t i = 0; i < num_buses; ++i) {
        int cur_y = 5;
        Fl_Box* b = new Fl_Box(20 + x_offset, cur_y, 80, std_h, strdup(("Bus " + ::std::to_string(i+1)).c_str()));
        b->labelsize(font_sz);
        cur_y += std_h + 5;
        
        Fl_Slider* vol = new Fl_Slider(20 + x_offset, cur_y, 20, 150);
        vol->type(FL_VERTICAL);
        vol->bounds(1, 0); 
        vol->value(m_engine.bus(i).volume());
        vol->callback(cb_bus_volume, new ::std::pair<MixerPanel*,int>(this, (int)i));

        VUMeter* meter_l = new VUMeter(45 + x_offset, cur_y, 8, 150);
        VUMeter* meter_r = new VUMeter(55 + x_offset, cur_y, 8, 150);
        m_bus_meters.push_back({meter_l, meter_r});
        cur_y += 150 + 5;

        Fl_Slider* pan_slider = new Fl_Slider(20 + x_offset, cur_y, 75, 15);
        pan_slider->type(FL_HOR_SLIDER);
        pan_slider->range(-1.0, 1.0);
        pan_slider->value(m_engine.bus(i).pan());
        pan_slider->callback(cb_bus_pan, new ::std::pair<MixerPanel*,int>(this, (int)i));
        cur_y += 15 + 5;

        Fl_Check_Button* mute = new Fl_Check_Button(20 + x_offset, cur_y, 35, std_h, "M");
        mute->labelsize(font_sz);
        mute->value(m_engine.bus(i).muted());
        mute->callback(cb_bus_mute, new ::std::pair<MixerPanel*,int>(this, (int)i));
        
        x_offset += 100;
      }
    }

    m_track_group->end();
    m_track_group->size((int)(x_offset + 40), m_track_group->h());
    m_track_group->redraw();
}

void MixerPanel::update_meters() {
    if (m_master_meter_l) m_master_meter_l->level(m_engine.master_meter_l());
    if (m_master_meter_r) m_master_meter_r->level(m_engine.master_meter_r());

    if (m_spectral_view) {
        m_spectral_view->update();
    }

    for (size_t i = 0; i < m_track_meters.size() && i < m_engine.track_count(); ++i) {
        m_track_meters[i].first->level(m_engine.track(i).meter_level_l());
        m_track_meters[i].second->level(m_engine.track(i).meter_level_r());
    }

    for (size_t i = 0; i < m_bus_meters.size() && i < m_engine.bus_count(); ++i) {
        m_bus_meters[i].first->level(m_engine.bus(i).meter_l());
        m_bus_meters[i].second->level(m_engine.bus(i).meter_r());
    }
}

void MixerPanel::update_effect_editor() {
    for (int i = 0; i < m_fx_chain_group->children(); ++i) {
        void* d = m_fx_chain_group->child(i)->user_data();
        if (d) delete static_cast<::std::pair<MixerPanel*,int>*>(d);
    }
    m_fx_chain_group->clear();
    
    for (int i = 0; i < m_fx_params_group->children(); ++i) {
        void* d = m_fx_params_group->child(i)->user_data();
        if (d) delete static_cast<::std::pair<MixerPanel*,int>*>(d);
    }
    m_fx_params_group->clear();

    if (m_selected_track < -1 || m_selected_track >= (int)m_engine.track_count()) return;

    bool is_master = (m_selected_track == -1);
    auto get_fx = [&](size_t idx) -> DSP* {
        if (is_master) return m_engine.m_master.get_effect(idx);
        return m_engine.track(m_selected_track).get_effect(idx);
    };

    if (m_selected_fx_slot != -1 && !get_fx(m_selected_fx_slot)) m_selected_fx_slot = -1;

    int std_h = m_engine.m_gui_button_height;
    int font_sz = m_engine.m_gui_font_size;
    int spacing = 2;

    m_fx_chain_group->begin();
    Fl_Scroll* chain_scroll = new Fl_Scroll(m_fx_chain_group->x(), m_fx_chain_group->y(), m_fx_chain_group->w(), m_fx_chain_group->h());
    chain_scroll->type(Fl_Scroll::VERTICAL);
    Fl_Pack* chain_pack = new Fl_Pack(chain_scroll->x(), chain_scroll->y(), chain_scroll->w() - 20, 1);
    chain_pack->type(Fl_Pack::VERTICAL);
    chain_pack->spacing(spacing);
    chain_pack->begin();

    for (size_t i = 0; i < MAX_INSERTS; ++i) {
        DSP* dsp = get_fx(i);
        if (dsp) {
            Fl_Group* row = new Fl_Group(0, 0, chain_pack->w(), std_h);
            row->begin();
            Fl_Check_Button* bypass = new Fl_Check_Button(row->x(), row->y(), 25, std_h);
            bypass->value(!dsp->is_bypassed());
            bypass->callback(cb_fx_bypass, new ::std::pair<MixerPanel*,int>(this, (int)i));
            Fl_Button* sel_btn = new Fl_Button(row->x() + 28, row->y(), 100, std_h, strdup(dsp->name().c_str()));
            sel_btn->labelsize(font_sz);
            if ((int)i == m_selected_fx_slot) sel_btn->color(FL_YELLOW);
            sel_btn->callback([](Fl_Widget* w, void* d){
                auto* p = static_cast<::std::pair<MixerPanel*,int>*>(d);
                p->first->m_selected_fx_slot = p->second;
                p->first->update_effect_editor();
            }, new ::std::pair<MixerPanel*,int>(this, (int)i));
            int bx = row->x() + 132, bw = 30;
            Fl_Button* up = new Fl_Button(bx, row->y(), bw, std_h, "@8");
            up->callback(cb_fx_up, new ::std::pair<MixerPanel*,int>(this, (int)i));
            Fl_Button* dn = new Fl_Button(bx + bw + 2, row->y(), bw, std_h, "@2");
            dn->callback(cb_fx_down, new ::std::pair<MixerPanel*,int>(this, (int)i));
            Fl_Button* rm = new Fl_Button(bx + 2*(bw + 2), row->y(), bw, std_h, "X");
            rm->callback(cb_fx_remove, new ::std::pair<MixerPanel*,int>(this, (int)i));
            row->end();
        }
    }
    chain_pack->end(); chain_scroll->end();
    m_fx_chain_group->end(); m_fx_chain_group->redraw();

    m_fx_params_group->begin();
    if (m_selected_fx_slot != -1) {
        DSP* dsp = get_fx(m_selected_fx_slot);
        if (dsp) {
            Fl_Scroll* param_scroll = new Fl_Scroll(m_fx_params_group->x(), m_fx_params_group->y(), m_fx_params_group->w(), m_fx_params_group->h());
            param_scroll->type(Fl_Scroll::VERTICAL);
            Fl_Pack* param_pack = new Fl_Pack(param_scroll->x(), param_scroll->y(), param_scroll->w() - 25, 1);
            param_pack->type(Fl_Pack::VERTICAL);
            param_pack->spacing(spacing * 2);
            param_pack->begin();
            Fl_Box* header = new Fl_Box(0, 0, param_pack->w(), std_h, strdup(("Editing: " + dsp->name()).c_str()));
            header->labelfont(FL_BOLD); header->labelsize(font_sz + 2); header->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            Fl_Group* pre_row = new Fl_Group(0, 0, param_pack->w(), std_h);
            pre_row->begin();
            new Fl_Box(pre_row->x(), pre_row->y(), 60, std_h, "Preset:");
            Fl_Choice* presets = new Fl_Choice(pre_row->x() + 65, pre_row->y(), 150, std_h);
            presets->labelsize(font_sz);
            for (const auto& p : dsp->get_presets()) presets->add(strdup(p.c_str()));
            presets->value(0); presets->callback(cb_fx_preset_select, new ::std::pair<MixerPanel*,int>(this, m_selected_fx_slot));
            Fl_Button* psave = new Fl_Button(pre_row->x() + 220, pre_row->y(), 60, std_h, "Save");
            psave->labelsize(font_sz); psave->callback(cb_fx_preset_save, new ::std::pair<MixerPanel*,int>(this, m_selected_fx_slot));
            Fl_Button* pload = new Fl_Button(pre_row->x() + 285, pre_row->y(), 60, std_h, "Load");
            pload->labelsize(font_sz); pload->callback(cb_fx_preset_load, new ::std::pair<MixerPanel*,int>(this, m_selected_fx_slot));
            pre_row->end();
            if (auto* g = dynamic_cast<GainDSP*>(dsp)) {
                Fl_Value_Slider* s = new Fl_Value_Slider(0, 0, param_pack->w(), std_h, "Gain");
                s->labelsize(font_sz); s->type(FL_HOR_NICE_SLIDER); s->range(0, 2); s->value(g->gain);
                s->callback([](Fl_Widget* w, void* d){ ((GainDSP*)d)->gain = (float)((Fl_Value_Slider*)w)->value(); }, g);
            } else if (auto* d = dynamic_cast<DelayDSP*>(dsp)) {
                Fl_Value_Slider* s1 = new Fl_Value_Slider(0, 0, param_pack->w(), std_h, "Feedback");
                s1->labelsize(font_sz); s1->type(FL_HOR_NICE_SLIDER); s1->range(0, 0.99); s1->value(d->feedback);
                s1->callback([](Fl_Widget* w, void* v){ ((DelayDSP*)v)->feedback = (float)((Fl_Value_Slider*)w)->value(); }, d);
                Fl_Value_Slider* s2 = new Fl_Value_Slider(0, 0, param_pack->w(), std_h, "Mix");
                s2->labelsize(font_sz); s2->type(FL_HOR_NICE_SLIDER); s2->range(0, 1); s2->value(d->mix);
                s2->callback([](Fl_Widget* w, void* v){ ((DelayDSP*)v)->mix = (float)((Fl_Value_Slider*)w)->value(); }, d);
            }
            param_pack->end(); param_scroll->end();
        }
    } else {
        Fl_Box* msg = new Fl_Box(m_fx_params_group->x(), m_fx_params_group->y(), m_fx_params_group->w(), m_fx_params_group->h(), "Select an effect to edit parameters");
        msg->labelsize(font_sz);
    }
    m_fx_params_group->end(); m_fx_params_group->redraw();
}

void MixerPanel::cb_track_select(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    pair->first->m_selected_track = pair->second;
    pair->first->m_selected_fx_slot = -1;
    pair->first->update_mixer_ui();
    pair->first->update_effect_editor();
}

void MixerPanel::cb_add_fx(Fl_Widget* w, void* data) {
    MixerPanel* self = static_cast<MixerPanel*>(data);
    int fx_idx = self->m_avail_fx_browser->value();
    if (fx_idx <= 0) return;
    std::string fx_name = self->m_avail_fx_browser->text(fx_idx);
    bool is_master = (self->m_selected_track == -1);
    auto get_fx_at = [&](size_t idx) -> DSP* {
        if (is_master) return self->m_engine.m_master.get_effect(idx);
        return self->m_engine.track(self->m_selected_track).get_effect(idx);
    };
    auto set_fx_at = [&](size_t idx, std::unique_ptr<DSP> dsp) {
        if (is_master) { self->m_engine.m_master.set_effect(idx, std::move(dsp)); self->m_engine.m_master.enable_effect(idx, true); }
        else { self->m_engine.track(self->m_selected_track).set_effect(idx, std::move(dsp)); self->m_engine.track(self->m_selected_track).enable_effect(idx, true); }
    };
    for (size_t i = 0; i < MAX_INSERTS; ++i) {
        DSP* dsp = get_fx_at(i);
        if (dsp && dsp->name() == fx_name) return;
    }
    for (size_t i = 0; i < MAX_INSERTS; ++i) {
        if (!get_fx_at(i)) {
            if (fx_name == "Gain") set_fx_at(i, std::make_unique<GainDSP>());
            else if (fx_name == "Delay") set_fx_at(i, std::make_unique<DelayDSP>());
            self->m_selected_fx_slot = (int)i;
            break;
        }
    }
    self->update_effect_editor();
}

void MixerPanel::cb_fx_up(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    if (pair->first->m_selected_track == -1) pair->first->m_engine.m_master.move_effect_up(pair->second);
    else pair->first->m_engine.track(pair->first->m_selected_track).move_effect_up(pair->second);
    pair->first->update_effect_editor();
}

void MixerPanel::cb_fx_down(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    if (pair->first->m_selected_track == -1) pair->first->m_engine.m_master.move_effect_down(pair->second);
    else pair->first->m_engine.track(pair->first->m_selected_track).move_effect_down(pair->second);
    pair->first->update_effect_editor();
}

void MixerPanel::cb_fx_remove(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    if (pair->first->m_selected_track == -1) pair->first->m_engine.m_master.remove_effect(pair->second);
    else pair->first->m_engine.track(pair->first->m_selected_track).remove_effect(pair->second);
    pair->first->update_effect_editor();
}

void MixerPanel::cb_fx_bypass(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    DSP* dsp = (pair->first->m_selected_track == -1) ? pair->first->m_engine.m_master.get_effect(pair->second) : pair->first->m_engine.track(pair->first->m_selected_track).get_effect(pair->second);
    if (dsp) dsp->set_bypass(!static_cast<Fl_Check_Button*>(w)->value());
}

void MixerPanel::cb_fx_preset_select(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    DSP* dsp = (pair->first->m_selected_track == -1) ? pair->first->m_engine.m_master.get_effect(pair->second) : pair->first->m_engine.track(pair->first->m_selected_track).get_effect(pair->second);
    if (dsp) dsp->load_preset(static_cast<Fl_Choice*>(w)->text());
    pair->first->update_effect_editor();
}

void MixerPanel::cb_fx_preset_save(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    DSP* dsp = (pair->first->m_selected_track == -1) ? pair->first->m_engine.m_master.get_effect(pair->second) : pair->first->m_engine.track(pair->first->m_selected_track).get_effect(pair->second);
    if (!dsp) return;
    Fl_Native_File_Chooser fnfc; fnfc.title("Save Preset"); fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE); fnfc.filter("JSON Files\t*.json\n");
    if (fnfc.show() == 0) { std::ofstream f(fnfc.filename()); if (f.is_open()) f << dsp->get_state(); }
}

void MixerPanel::cb_fx_preset_load(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    DSP* dsp = (pair->first->m_selected_track == -1) ? pair->first->m_engine.m_master.get_effect(pair->second) : pair->first->m_engine.track(pair->first->m_selected_track).get_effect(pair->second);
    if (!dsp) return;
    Fl_Native_File_Chooser fnfc; fnfc.title("Load Preset"); fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE); fnfc.filter("JSON Files\t*.json\n");
    if (fnfc.show() == 0) { std::ifstream f(fnfc.filename()); if (f.is_open()) { std::stringstream buffer; buffer << f.rdbuf(); dsp->set_state(buffer.str()); pair->first->update_effect_editor(); } }
}

void MixerPanel::cb_save_chain(Fl_Widget* w, void* data) {
    MixerPanel* self = static_cast<MixerPanel*>(data);
    Fl_Native_File_Chooser fnfc; fnfc.title("Save Effect Chain"); fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE); fnfc.filter("Chain Files\t*.chain\n");
    if (fnfc.show() == 0) { if (self->m_selected_track == -1) self->m_engine.m_master.save_effect_chain(fnfc.filename()); else self->m_engine.track(self->m_selected_track).save_effect_chain(fnfc.filename()); }
}

void MixerPanel::cb_load_chain(Fl_Widget* w, void* data) {
    MixerPanel* self = static_cast<MixerPanel*>(data);
    Fl_Native_File_Chooser fnfc; fnfc.title("Load Effect Chain"); fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE); fnfc.filter("Chain Files\t*.chain\n");
    if (fnfc.show() == 0) { if (self->m_selected_track == -1) self->m_engine.m_master.load_effect_chain(fnfc.filename()); else self->m_engine.track(self->m_selected_track).load_effect_chain(fnfc.filename()); self->m_selected_fx_slot = -1; self->update_effect_editor(); }
}

void MixerPanel::cb_master_gain(Fl_Widget* w, void* data) {
    MixerPanel* self = static_cast<MixerPanel*>(data);
    float g = static_cast<Fl_Value_Slider*>(w)->value();
    self->m_engine.set_master_gain(g);
}

void MixerPanel::cb_track_volume(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    pair->first->m_engine.track(pair->second).set_volume(static_cast<Fl_Slider*>(w)->value());
}

void MixerPanel::cb_track_pan(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    pair->first->m_engine.track(pair->second).set_pan(static_cast<Fl_Slider*>(w)->value());
}

void MixerPanel::cb_track_mute(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    pair->first->m_engine.track(pair->second).set_mute(static_cast<Fl_Check_Button*>(w)->value());
}

void MixerPanel::cb_track_solo(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    pair->first->m_engine.track(pair->second).set_solo(static_cast<Fl_Check_Button*>(w)->value());
}

void MixerPanel::cb_track_input(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    int val = static_cast<Fl_Choice*>(w)->value(); uint32_t num_ins = pair->first->m_engine.m_num_ins;
    if (val == 0) pair->first->m_engine.track(pair->second).set_audio_input(-1, -1);
    else if (val <= (int)num_ins) pair->first->m_engine.track(pair->second).set_audio_input(val - 1, -1);
    else { int idx = val - (int)num_ins - 1; pair->first->m_engine.track(pair->second).set_audio_input(idx * 2, idx * 2 + 1); }
}

void MixerPanel::cb_track_delay(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    pair->first->m_engine.track(pair->second).set_input_delay((float)static_cast<Fl_Value_Input*>(w)->value(), pair->first->m_engine.sample_rate());
}

void MixerPanel::cb_bus_volume(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    pair->first->m_engine.bus(pair->second).set_volume(static_cast<Fl_Slider*>(w)->value());
}

void MixerPanel::cb_bus_pan(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    pair->first->m_engine.bus(pair->second).set_pan(static_cast<Fl_Slider*>(w)->value());
}

void MixerPanel::cb_bus_mute(Fl_Widget* w, void* data) {
    auto* pair = static_cast<::std::pair<MixerPanel*,int>*>(data);
    pair->first->m_engine.bus(pair->second).set_mute(static_cast<Fl_Check_Button*>(w)->value());
}

void MixerPanel::cb_detach(Fl_Widget*, void* data) {
    MixerPanel* self = static_cast<MixerPanel*>(data);
    if (self->m_detached_window) self->m_detached_window->show();
    else { self->m_detached_window = new DetachedWindow(850, 300, "Mixer", self, self->parent()); self->m_detached_window->show(); }
    self->hide();
}

} // namespace disgrace_ns
