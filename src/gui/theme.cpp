#include "theme.h"
#include "../core/engine.h"
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>

namespace disgrace_ns {

std::vector<Theme> ThemeManager::m_themes;
ThemeType ThemeManager::m_current_theme = ThemeType::Classic;
bool ThemeManager::m_initialized = false;

void ThemeManager::apply_theme_and_settings(Engine& engine) {
    init_themes();
    ThemeType type = engine.m_gui_theme;
    m_current_theme = type;
    
    const Theme* theme = nullptr;
    size_t idx = static_cast<size_t>(type);
    if (idx < m_themes.size()) {
        theme = &m_themes[idx];
    }

    if (theme) {
        // Sync engine with theme defaults if it's not a custom theme with overrides already set
        // Usually, applying a theme means we want its colors
        engine.m_bg_color = (unsigned int)theme->background;
        engine.m_fg_color = (unsigned int)theme->foreground;
        engine.m_boxtype = (int)theme->boxtype;
        engine.m_btn_boxtype = (int)theme->button_boxtype;
        engine.m_button_color = (unsigned int)theme->button_color;

        // Sync tracker colors
        engine.m_tracker_bg = (unsigned int)theme->tracker_bg;
        engine.m_tracker_text = (unsigned int)theme->tracker_text;
        engine.m_tracker_cursor = (unsigned int)theme->tracker_cursor;
        engine.m_tracker_row_highlight = (unsigned int)theme->tracker_row_highlight;
        engine.m_tracker_lpb_highlight = (unsigned int)theme->tracker_lpb_highlight;
        engine.m_tracker_note = (unsigned int)theme->tracker_note;
        engine.m_tracker_sample = (unsigned int)theme->tracker_sample;
        engine.m_tracker_volume = (unsigned int)theme->tracker_volume;
        engine.m_tracker_effect = (unsigned int)theme->tracker_effect;
        
        Fl::scheme(theme->fl_scheme);
    }

    apply_gui_settings(engine.m_gui_font_size, engine.m_gui_button_height);
    
    // Force colors from engine to FLTK globals
    unsigned char r, g, b;
    Fl::get_color((Fl_Color)engine.m_bg_color, r, g, b);
    Fl::background(r, g, b);
    
    Fl_Color bg2 = (Fl_Color)engine.m_bg_color;
    if (theme) bg2 = theme->input_background;
    Fl::get_color(bg2, r, g, b);
    Fl::background2(r, g, b);

    Fl::get_color((Fl_Color)engine.m_fg_color, r, g, b);
    Fl::foreground(r, g, b);

    if (theme) {
        Fl::set_color(FL_SELECTION_COLOR, theme->selection);
    }

    // Re-apply to all widgets to ensure boxtypes and button colors are set
    for (Fl_Window* win = Fl::first_window(); win; win = Fl::next_window(win)) {
        apply_engine_to_widgets(win, engine);
        win->redraw();
    }
}

void ThemeManager::apply_engine_to_widgets(::Fl_Widget* w, Engine& engine) {
    if (!w) return;

    // Apply button specific settings
    if (dynamic_cast<Fl_Button*>(w)) {
        w->box((Fl_Boxtype)engine.m_btn_boxtype);
        w->color((Fl_Color)engine.m_button_color);
    } else if (dynamic_cast<Fl_Window*>(w)) {
        // Windows usually keep background
    } else {
        // Other widgets get general boxtype
        w->box((Fl_Boxtype)engine.m_boxtype);
    }

    // Label box type
    w->labelcolor((Fl_Color)engine.m_fg_color);
    w->labelfont(FL_HELVETICA);
    // w->labeltype(FL_NORMAL_LABEL); // Default

    // If it's a group, recurse
    Fl_Group* g = w->as_group();
    if (g) {
        for (int i = 0; i < g->children(); i++) {
            apply_engine_to_widgets(g->child(i), engine);
        }
    }
}

void ThemeManager::apply_to_all_widgets(::Fl_Widget* w, int font_size, int btn_h) {
    if (!w) return;
    w->labelsize(font_size);
    
    // If it's a button, try to resize it (height only)
    if (dynamic_cast<Fl_Button*>(w)) {
        w->size(w->w(), btn_h);
    }

    // If it's a group, recurse into children
    Fl_Group* g = w->as_group();
    if (g) {
        for (int i = 0; i < g->children(); i++) {
            apply_to_all_widgets(g->child(i), font_size, btn_h);
        }
    }
}

void ThemeManager::apply_gui_settings(int font_size, int button_height) {
    FL_NORMAL_SIZE = font_size;
    
    for (Fl_Window* win = Fl::first_window(); win; win = Fl::next_window(win)) {
        apply_to_all_widgets(win, font_size, button_height);
        win->redraw();
    }
}

static void set_tracker_colors(Theme& t, Fl_Color bg, Fl_Color text) {
    t.tracker_bg = bg;
    t.tracker_text = text;
    t.tracker_cursor = fl_rgb_color(255, 255, 0);
    t.tracker_row_highlight = fl_rgb_color(60, 60, 80);
    t.tracker_lpb_highlight = fl_rgb_color(45, 45, 55);
    t.tracker_note = fl_rgb_color(180, 180, 255);
    t.tracker_sample = fl_rgb_color(180, 255, 180);
    t.tracker_volume = fl_rgb_color(255, 180, 180);
    t.tracker_effect = fl_rgb_color(255, 255, 180);
}

void ThemeManager::init_themes() {
    if (m_initialized) return;
    m_themes.clear();

    // Light
    Theme light;
    light.name = "Light";
    light.background = fl_rgb_color(240, 240, 240);
    light.foreground = FL_BLACK;
    light.selection = fl_rgb_color(180, 210, 255);
    light.inactive = fl_rgb_color(200, 200, 200);
    light.boxtype = FL_FLAT_BOX;
    light.button_boxtype = FL_UP_BOX;
    light.label_boxtype = FL_NO_BOX;
    light.button_color = fl_rgb_color(220, 220, 220);
    light.input_background = FL_WHITE;
    light.text_color = FL_BLACK;
    light.label_color = FL_BLACK;
    light.fl_scheme = "gleam";
    set_tracker_colors(light, fl_rgb_color(250, 250, 250), FL_BLACK);
    light.tracker_row_highlight = fl_rgb_color(220, 220, 240);
    light.tracker_lpb_highlight = fl_rgb_color(235, 235, 245);
    m_themes.push_back(light);

    // Dark
    Theme dark;
    dark.name = "Dark";
    dark.background = fl_rgb_color(45, 45, 45);
    dark.foreground = fl_rgb_color(220, 220, 220);
    dark.selection = fl_rgb_color(70, 70, 100);
    dark.inactive = fl_rgb_color(80, 80, 80);
    dark.boxtype = FL_FLAT_BOX;
    dark.button_boxtype = FL_UP_BOX;
    dark.label_boxtype = FL_NO_BOX;
    dark.button_color = fl_rgb_color(60, 60, 60);
    dark.input_background = fl_rgb_color(30, 30, 30);
    dark.text_color = fl_rgb_color(220, 220, 220);
    dark.label_color = fl_rgb_color(220, 220, 220);
    dark.fl_scheme = "gtk+";
    set_tracker_colors(dark, fl_rgb_color(30, 30, 30), fl_rgb_color(220, 220, 220));
    m_themes.push_back(dark);

    // Pastel
    Theme pastel;
    pastel.name = "Pastel Green";
    pastel.background = fl_rgb_color(230, 245, 230);
    pastel.foreground = fl_rgb_color(60, 60, 80);
    pastel.selection = fl_rgb_color(255, 220, 220);
    pastel.inactive = fl_rgb_color(210, 220, 210);
    pastel.boxtype = FL_ROUNDED_BOX;
    pastel.button_boxtype = FL_ROUNDED_BOX;
    pastel.label_boxtype = FL_ROUNDED_BOX;
    pastel.button_color = fl_rgb_color(210, 235, 210);
    pastel.input_background = fl_rgb_color(250, 255, 250);
    pastel.text_color = fl_rgb_color(60, 60, 80);
    pastel.label_color = fl_rgb_color(40, 40, 60);
    pastel.fl_scheme = "plastic";
    set_tracker_colors(pastel, fl_rgb_color(245, 255, 245), fl_rgb_color(60, 60, 80));
    pastel.tracker_row_highlight = fl_rgb_color(220, 240, 220);
    m_themes.push_back(pastel);

    // Pastel Cyan
    Theme pcyan = pastel;
    pcyan.name = "Pastel Cyan";
    pcyan.background = fl_rgb_color(230, 245, 245);
    pcyan.button_color = fl_rgb_color(210, 235, 235);
    pcyan.input_background = fl_rgb_color(250, 255, 255);
    set_tracker_colors(pcyan, fl_rgb_color(245, 255, 255), fl_rgb_color(60, 60, 80));
    pcyan.tracker_row_highlight = fl_rgb_color(220, 240, 240);
    m_themes.push_back(pcyan);

    // Pastel Blue
    Theme pblue = pastel;
    pblue.name = "Pastel Blue";
    pblue.background = fl_rgb_color(230, 235, 245);
    pblue.button_color = fl_rgb_color(210, 215, 235);
    pblue.input_background = fl_rgb_color(250, 252, 255);
    set_tracker_colors(pblue, fl_rgb_color(245, 248, 255), fl_rgb_color(60, 60, 80));
    pblue.tracker_row_highlight = fl_rgb_color(220, 225, 240);
    m_themes.push_back(pblue);

    // Pastel Gray
    Theme pgray = pastel;
    pgray.name = "Pastel Gray";
    pgray.background = fl_rgb_color(235, 235, 235);
    pgray.button_color = fl_rgb_color(220, 220, 220);
    pgray.input_background = fl_rgb_color(250, 250, 250);
    set_tracker_colors(pgray, fl_rgb_color(245, 245, 245), fl_rgb_color(60, 60, 80));
    pgray.tracker_row_highlight = fl_rgb_color(225, 225, 225);
    m_themes.push_back(pgray);

    // Pastel Red
    Theme pred = pastel;
    pred.name = "Pastel Red";
    pred.background = fl_rgb_color(245, 235, 235);
    pred.button_color = fl_rgb_color(235, 220, 220);
    pred.input_background = fl_rgb_color(255, 250, 250);
    set_tracker_colors(pred, fl_rgb_color(255, 245, 245), fl_rgb_color(60, 60, 80));
    pred.tracker_row_highlight = fl_rgb_color(240, 220, 220);
    m_themes.push_back(pred);

    // Classic (Standard FLTK)
    Theme classic;
    classic.name = "Classic";
    classic.background = FL_GRAY;
    classic.foreground = FL_BLACK;
    classic.selection = FL_SELECTION_COLOR;
    classic.inactive = FL_INACTIVE_COLOR;
    classic.boxtype = FL_UP_BOX;
    classic.button_boxtype = FL_UP_BOX;
    classic.label_boxtype = FL_NO_BOX;
    classic.button_color = FL_GRAY;
    classic.input_background = FL_WHITE;
    classic.text_color = FL_BLACK;
    classic.label_color = FL_BLACK;
    classic.fl_scheme = "none";
    set_tracker_colors(classic, FL_WHITE, FL_BLACK);
    classic.tracker_bg = fl_rgb_color(30, 30, 30); // Use traditional dark tracker by default
    classic.tracker_text = fl_rgb_color(200, 200, 200);
    m_themes.push_back(classic);

    // Plastic, GTK, Gleam can be added as variants or just schemes

    // Custom
    Theme custom;
    custom.name = "Custom";
    custom.background = FL_GRAY;
    custom.foreground = FL_BLACK;
    custom.selection = FL_SELECTION_COLOR;
    custom.inactive = FL_INACTIVE_COLOR;
    custom.boxtype = FL_UP_BOX;
    custom.button_boxtype = FL_UP_BOX;
    custom.label_boxtype = FL_NO_BOX;
    custom.button_color = FL_GRAY;
    custom.input_background = FL_WHITE;
    custom.text_color = FL_BLACK;
    custom.label_color = FL_BLACK;
    custom.fl_scheme = "none";
    set_tracker_colors(custom, fl_rgb_color(30, 30, 30), fl_rgb_color(200, 200, 200));
    m_themes.push_back(custom);

    m_initialized = true;
}

void ThemeManager::apply_theme(ThemeType type) {
    init_themes();
    m_current_theme = type;
    size_t idx = static_cast<size_t>(type);
    if (idx < m_themes.size()) {
        apply_theme(m_themes[idx]);
    }
}

void ThemeManager::apply_theme(const Theme& theme) {
    Fl::scheme(theme.fl_scheme);
    
    // Set colors
    unsigned char r, g, b;
    
    // Background
    Fl::get_color(theme.background, r, g, b);
    Fl::background(r, g, b);
    
    // Background2 (for inputs, lists)
    Fl::get_color(theme.input_background, r, g, b);
    Fl::background2(r, g, b);
    
    // Foreground
    Fl::get_color(theme.foreground, r, g, b);
    Fl::foreground(r, g, b);
    
    // Selection
    Fl::set_color(FL_SELECTION_COLOR, theme.selection);
    
    // We can also iterate over all windows and widgets to apply changes
    for (Fl_Window* win = Fl::first_window(); win; win = Fl::next_window(win)) {
        win->redraw();
    }
}

const std::vector<Theme>& ThemeManager::get_available_themes() {
    init_themes();
    return m_themes;
}

int ThemeManager::get_boxtype_index(int boxtype) {
    static const int bt[] = { 
        FL_NO_BOX, FL_FLAT_BOX, FL_UP_BOX, FL_DOWN_BOX, FL_BORDER_BOX, 
        FL_SHADOW_BOX, FL_ROUNDED_BOX, FL_PLASTIC_UP_BOX, FL_GLEAM_UP_BOX 
    };
    for (int i = 0; i < 9; i++) {
        if (bt[i] == boxtype) return i;
    }
    return 2; // Default to FL_UP_BOX
}

int ThemeManager::get_boxtype_at_index(int index) {
    static const int bt[] = { 
        FL_NO_BOX, FL_FLAT_BOX, FL_UP_BOX, FL_DOWN_BOX, FL_BORDER_BOX, 
        FL_SHADOW_BOX, FL_ROUNDED_BOX, FL_PLASTIC_UP_BOX, FL_GLEAM_UP_BOX 
    };
    if (index >= 0 && index < 9) return bt[index];
    return FL_UP_BOX;
}

} // namespace disgrace_ns
