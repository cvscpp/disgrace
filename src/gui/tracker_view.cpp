#include "tracker_view.h"
#include <FL/fl_draw.H>
#include "../core/engine.h" 
#include <FL/Fl.H>
#include "../edit/cmd_clear_block.h"
#include "../edit/cmd_set_note.h"
#include "../edit/cmd_paste_block.h"
#include "../core/key_bindings.h"

namespace disgrace_ns
{

    static int action_to_note(Action action)
    {
        switch (action)
        {
            case Action::NoteC:  return 48;
            case Action::NoteCs: return 49;
            case Action::NoteD:  return 50;
            case Action::NoteDs: return 51;
            case Action::NoteE:  return 52;
            case Action::NoteF:  return 53;
            case Action::NoteFs: return 54;
            case Action::NoteG:  return 55;
            case Action::NoteGs: return 56;
            case Action::NoteA:  return 57;
            case Action::NoteAs: return 58;
            case Action::NoteB:  return 59;
            default: return -1;
        }
    }
    static const char* note_name(uint8_t note)
    {
        static const char* names[] =
        {"C","C#","D","D#","E","F",
            "F#","G","G#","A","A#","B"};

            static char buf[8];
            int octave = note / 12;
            int n = note % 12; // FIX: n = note % 12;

            snprintf(buf, sizeof(buf),
                     "%s%d", names[n], octave);
            return buf;
    }


    // Constructor definition
    disgrace_ns::TrackerView::TrackerView(int x, int y, int w, int h,
                                         Pattern& pattern,
                                         Engine& engine)
    : Fl_Widget(x, y, w, h),
      m_pattern(pattern),
      m_engine(engine)
    {

    }

    void disgrace_ns::TrackerView::draw()
    {

        size_t playing_row = m_engine.current_row();

        fl_push_clip(x(), y(), w(), h());

        fl_color(30, 30, 30);
        fl_rectf(x(), y(), w(), h());

        int row_height = 18;
        int col_width  = w() / m_tracks;
        int visible_rows = h() / row_height;

        fl_color(60, 60, 60);

        if (m_selecting)
        {
            int r0 = ::std::min(m_sel_row_start,
                              m_sel_row_end);
            int r1 = ::std::max(m_sel_row_start,
                              m_sel_row_end);

            int t0 = ::std::min(m_sel_track_start,
                              m_sel_track_end);
            int t1 = ::std::max(m_sel_track_start,
                              m_sel_track_end);

            fl_color(60, 60, 120);

            for (int t = t0; t <= t1; ++t)
            {
                for (int r = r0; r <= r1; ++r)
                {
                    fl_rectf(
                        x() + t * col_width,
                             y() + r * row_height,
                             col_width,
                             row_height);
                }
            }
        }

        // horizontal lines
        for (int r = 0; r <= m_rows; ++r)
        {
            // const auto& ev = // Commented out
            // m_pattern.track(t).row(r); // Commented out

            // if (ev.note != 255) // Commented out
            // { // Commented out
            // fl_draw(note_name(ev.note), // Commented out
            //         x() + t*col_width + 5, // Commented out
            //         y() + r*row_height + 14); // Commented out
            // } // Commented out

            if (m_engine.is_playing())
            {
                int pr = m_engine.current_row();
                if (pr < m_cursor_row - 10 ||
                    pr > m_cursor_row + 10)
                {
                    m_cursor_row = pr;
                }
            }

            if (r == playing_row &&
                m_engine.is_playing())
            {
                fl_color(50, 80, 120);
                fl_rectf(x(), y() + r*row_height,
                         w(), row_height);
            } else {

            int yy = y() + r * row_height;
            fl_line(x(), yy, x() + w(), yy);
            }
            fl_color(200, 200, 0);
            fl_rect(x() + m_cursor_track * col_width,
                    y() + m_cursor_row * row_height,
                    col_width,
                    row_height);

        }

        // vertical lines
        for (int t = 0; t <= m_tracks; ++t)
        {
            int xx = x() + t * col_width;
            fl_line(xx, y(), xx, y() + h());
        }

        fl_pop_clip();
    }

    int disgrace_ns::TrackerView::handle(int event)
    {
        bool shift = Fl::event_state(FL_SHIFT);
        int col_width  = w() / m_tracks; // ADDED
        int row_height = 18; // ADDED

        if (event == FL_PUSH)
        {
            int tx = (Fl::event_x() - x()) / col_width; // Corrected
            int local_x = Fl::event_x() - (x() + tx * col_width); // Corrected


            // Mute region
            if (local_x > 40 && local_x < 60)
            {
                bool m =
                !m_engine.track(tx).muted();

                m_engine.track(tx).set_mute(m);
                redraw();
                return 1;
            }

            // Solo region
            if (local_x > 65 && local_x < 85)
            {
                bool s =
                !m_engine.track(tx).solo();

                m_engine.track(tx).set_solo(s);
                redraw();
                return 1;
            }
        }


        switch (event)
        {
            case FL_KEYDOWN:
            {
                int key = Fl::event_key();
                int mods = Fl::event_state() & (FL_CTRL | FL_SHIFT | FL_ALT | FL_META);
                Action action = m_engine.m_key_bindings.get_action(key, mods);
                
                // If not found with mods, try without (for navigation with shift)
                if (action == static_cast<Action>(-1) && (mods & FL_SHIFT)) {
                    action = m_engine.m_key_bindings.get_action(key, mods & ~FL_SHIFT);
                }

                if (action == Action::Clear)
                {
                    auto cmd =
                    ::std::make_unique<disgrace_ns::CmdClearBlock>(
                        m_engine.pattern(),
                        m_sel_track_start, m_sel_track_end,
                        m_sel_row_start, m_sel_row_end);

                    m_engine.undo_stack()
                    .execute(static_cast<std::unique_ptr<disgrace_ns::Command>>(::std::move(cmd)));

                    m_selecting = false;
                    redraw();
                    return 1;
                }

                if (action == Action::MoveUp)
                {
                    if (shift && !m_selecting)
                    {
                        m_selecting = true;
                        m_sel_row_start = m_cursor_row;
                        m_sel_track_start = m_cursor_track;
                    }

                    m_cursor_row =
                    ::std::max(0, m_cursor_row - 1);

                    if (shift)
                    {
                        m_sel_row_end = m_cursor_row;
                        m_sel_track_end = m_cursor_track;
                    }
                    else
                    {
                        m_selecting = false;
                    }

                    redraw();
                    return 1;
                }

                if (action == Action::MoveDown)
                {
                    if (shift && !m_selecting)
                    {
                        m_selecting = true;
                        m_sel_row_start = m_cursor_row;
                        m_sel_track_start = m_cursor_track;
                    }

                    m_cursor_row =
                    ::std::min(m_rows - 1,
                             m_cursor_row + 1);

                    if (shift)
                    {
                        m_sel_row_end = m_cursor_row;
                        m_sel_track_end = m_cursor_track;
                    }
                    else
                    {
                        m_selecting = false;
                    }

                    redraw();
                    return 1;
                }

                if (action == Action::MoveLeft)
                {
                    if (shift && !m_selecting)
                    {
                        m_selecting = true;
                        m_sel_row_start = m_cursor_row;
                        m_sel_track_start = m_cursor_track;
                    }

                    m_cursor_track =
                    ::std::max(0, m_cursor_track - 1);

                    if (shift)
                    {
                        m_sel_row_end = m_cursor_row;
                        m_sel_track_end = m_cursor_track;
                    }
                    else
                    {
                        m_selecting = false;
                    }
                    redraw();
                    return 1;
                }

                if (action == Action::MoveRight)
                {
                    if (shift && !m_selecting)
                    {
                        m_selecting = true;
                        m_sel_row_start = m_cursor_row;
                        m_sel_track_start = m_cursor_track;
                    }

                    m_cursor_track =
                    ::std::min(m_tracks - 1,
                             m_cursor_track + 1);

                    if (shift)
                    {
                        m_sel_row_end = m_cursor_row;
                        m_sel_track_end = m_cursor_track;
                    }
                    else
                    {
                        m_selecting = false;
                    }

                    redraw();
                    return 1;
                }

                if (action == Action::Play)
                {
                    m_engine.toggle_play();
                    redraw();
                    return 1;
                }

                if (action == Action::Undo)
                {
                    m_engine.undo_stack().undo();
                    redraw();
                    return 1;
                }

                if (action == Action::Redo)
                {
                    m_engine.undo_stack().redo();
                    redraw();
                    return 1;
                }

                if (action == Action::Record)
                {
                    m_engine.record();
                    return 1;
                }

                if (action == Action::Copy)
                {
                    if (!m_selecting) return 1;

                    auto& pat = m_engine.pattern();
                    auto& cb  = m_engine.clipboard();

                    int r0 = ::std::min(m_sel_row_start,
                                      m_sel_row_end);
                    int r1 = ::std::max(m_sel_row_start,
                                      m_sel_row_end);

                    int t0 = ::std::min(m_sel_track_start,
                                      m_sel_track_end);
                    int t1 = ::std::max(m_sel_track_start,
                                      m_sel_track_end);

                    cb.width  = t1 - t0 + 1;
                    cb.height = r1 - r0 + 1;
                    cb.notes.clear();

                    for (int t_idx = t0; t_idx <= t1; ++t_idx)
                        for (int r_idx = r0; r_idx <= r1; ++r_idx)
                            cb.notes.push_back(
                                pat.event(t_idx, r_idx, 0).note);

                    return 1;
                }

                if (action == Action::Cut)
                {
                    if (!m_selecting) return 1;

                    auto& pat = m_engine.pattern();
                    auto& cb  = m_engine.clipboard();

                    int r0 = ::std::min(m_sel_row_start, m_sel_row_end);
                    int r1 = ::std::max(m_sel_row_start, m_sel_row_end);
                    int t0 = ::std::min(m_sel_track_start, m_sel_track_end);
                    int t1 = ::std::max(m_sel_track_start, m_sel_track_end);

                    // TODO: Implement copy logic here or reuse

                    auto clear =
                    ::std::make_unique<disgrace_ns::CmdClearBlock>(
                        pat,
                        t0,t1,r0,r1);

                    ::std::vector<disgrace_ns::EditCommandPtr> group;
                    group.push_back(static_cast<disgrace_ns::EditCommandPtr>(::std::move(clear)));

                    m_engine.undo_stack()
                    .execute_group(::std::move(group));

                    m_selecting = false;
                    redraw();
                    return 1;
                }

                if (action == Action::Paste)
                {
                    auto& cb = m_engine.clipboard();
                    if (cb.width == 0) return 1;

                    auto& pat = m_engine.pattern();

                    auto cmd =
                    ::std::make_unique<disgrace_ns::CmdPasteBlock>(
                        pat,
                        cb,
                        m_cursor_track,
                        m_cursor_row);

                    m_engine.undo_stack()
                    .execute(static_cast<std::unique_ptr<disgrace_ns::Command>>(::std::move(cmd)));

                    redraw();
                    return 1;
                }

                int note = action_to_note(action);
                if (note >= 0)
                {
                    insert_note(note);
                    return 1;
                }
                break;
            }
        }

        return Fl_Widget::handle(event);
    }

    void disgrace_ns::TrackerView::insert_note(uint8_t note)
    {
        auto cmd =
        ::std::make_unique<disgrace_ns::CmdSetNote>(
            m_engine.pattern(),
            m_cursor_track,
            m_cursor_row,
            note);

        m_engine.undo_stack()
        .execute(static_cast<std::unique_ptr<disgrace_ns::Command>>(::std::move(cmd)));

        m_engine.preview_note(
            m_cursor_track,
            note);

        m_cursor_row++;
        redraw();
    }

    void disgrace_ns::TrackerView::set_current_row(int row)
    {
        m_cursor_row = row;
        redraw();
    }

    // The second implementation of handle was present from lines 498-542
    // It is now the correct one.

    void disgrace_ns::TrackerView::draw_track_header( // Corrected from TrackerWidget
        int track_index,
        int x, int y,
        int w, int h)
    {
        auto& track =
        m_engine.track(track_index);

        fl_color(FL_DARK3);
        fl_rectf(x, y, w, h);

        // Track number
        fl_color(FL_WHITE);
        fl_draw(
            ("T" + ::std::to_string(track_index)).c_str(),
                x + 5, y + 15);

        // Mute button
        int btn_w = 20;
        int btn_h = 15;

        int mx = x + 40;
        int my = y + 5;

        fl_color(track.muted() ? FL_RED : FL_GRAY);
        fl_rectf(mx, my, btn_w, btn_h);
        fl_color(FL_WHITE);
        fl_draw("M", mx+6, my+12);

        // Solo button
        int sx = mx + 25;

        fl_color(track.solo() ? FL_GREEN : FL_GRAY);
        fl_rectf(sx, my, btn_w, btn_h);
        fl_color(FL_WHITE);
        fl_draw("S", sx+6, my+12);

        // Meter
        float level =
        track.meter_level();

        int meter_h =
        static_cast<int>(
            level * (h - 25));

        fl_color(FL_GREEN);
        fl_rectf(
            x + w - 10,
            y + h - meter_h,
            6,
            meter_h);
    }

} // namespace disgrace_ns
