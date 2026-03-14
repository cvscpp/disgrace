#ifndef DISGRACE_GUI_THEME_H
#define DISGRACE_GUI_THEME_H

#include <FL/Enumerations.H>
#include <vector>
#include <string>

class Fl_Widget;

namespace disgrace_ns {

enum class ThemeType {
    Light,
    Dark,
    Pastel,
    PastelCyan,
    PastelBlue,
    PastelGreen,
    PastelGray,
    PastelRed,
    Classic,
    Plastic,
    GTK,
    Gleam,
    Custom
};

struct Theme {
    std::string name;
    Fl_Color background;
    Fl_Color foreground;
    Fl_Color selection;
    Fl_Color inactive;
    Fl_Boxtype boxtype;
    Fl_Boxtype button_boxtype;
    Fl_Boxtype label_boxtype;
    Fl_Color button_color;
    Fl_Color input_background;
    Fl_Color text_color;
    Fl_Color label_color;
    const char* fl_scheme; // "none", "plastic", "gtk+", "gleam"

    // Tracker colors
    Fl_Color tracker_bg;
    Fl_Color tracker_text;
    Fl_Color tracker_cursor;
    Fl_Color tracker_row_highlight; // current playing row
    Fl_Color tracker_lpb_highlight; // every 4th/lpb row
    Fl_Color tracker_note;
    Fl_Color tracker_sample;
    Fl_Color tracker_volume;
    Fl_Color tracker_effect;
};

class ThemeManager {
public:
    static void apply_theme(ThemeType type);
    static void apply_theme(const Theme& theme);
    static void apply_gui_settings(int font_size, int button_height);
    static void apply_theme_and_settings(class Engine& engine);
    static const std::vector<Theme>& get_available_themes();
    static ThemeType get_current_theme_type() { return m_current_theme; }

    static int get_boxtype_index(int boxtype);
    static int get_boxtype_at_index(int index);

private:
    static void apply_engine_to_widgets(::Fl_Widget* w, class Engine& engine);
    static void apply_to_all_widgets(::Fl_Widget* w, int font_size, int btn_h);
    static void init_themes();
    static std::vector<Theme> m_themes;
    static ThemeType m_current_theme;
    static bool m_initialized;
};

} // namespace disgrace_ns

#endif // DISGRACE_GUI_THEME_H
