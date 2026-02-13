#include "tracker_view.h"
#include <FL/fl_draw.H>

namespace dg
{

    static int key_to_note(int key)
    {
        switch (key)
        {
            case 'z': return 48;
            case 's': return 49;
            case 'x': return 50;
            case 'd': return 51;
            case 'c': return 52;
            case 'v': return 53;
            case 'g': return 54;
            case 'b': return 55;
            case 'h': return 56;
            case 'n': return 57;
            case 'j': return 58;
            case 'm': return 59;
        }
        return -1;
    }
    static const char* note_name(uint8_t note)
    {
        static const char* names[] =
        {"C","C#","D","D#","E","F",
            "F#","G","G#","A","A#","B"};

            static char buf[8];
            int octave = note / 12;
            int n = note % 12;

            snprintf(buf, sizeof(buf),
                     "%s%d", names[n], octave);
            return buf;
    }


    TrackerView::TrackerView(int x, int y, int w, int h)
    : Fl_Widget(x, y, w, h)
    {

    }

    void TrackerView::draw()
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
            int r0 = std::min(m_sel_row_start,
                              m_sel_row_end);
            int r1 = std::max(m_sel_row_start,
                              m_sel_row_end);

            int t0 = std::min(m_sel_track_start,
                              m_sel_track_end);
            int t1 = std::max(m_sel_track_start,
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
            const auto& ev =
            m_pattern.track(t).row(r);

            if (ev.note != 255)
            {
                fl_draw(note_name(ev.note),
                        x() + t*col_width + 5,
                        y() + r*row_height + 14);
            }

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

    int TrackerView::handle(int event)
    {
        bool shift = Fl::event_state(FL_SHIFT);

        switch (event)
        {
            case FL_KEYDOWN:
            {
                int key = Fl::event_key();

                if (key == FL_Delete || key == FL_BackSpace)
                {
                    auto& pat = m_engine.pattern();

                    if (m_selecting)
                    {
                        int r0 = std::min(m_sel_row_start,
                                          m_sel_row_end);
                        int r1 = std::max(m_sel_row_start,
                                          m_sel_row_end);

                        int t0 = std::min(m_sel_track_start,
                                          m_sel_track_end);
                        int t1 = std::max(m_sel_track_start,
                                          m_sel_track_end);

                        auto cmd =
                        std::make_unique<CmdClearBlock>(
                            pat,
                            t0, t1,
                            r0, r1);

                        m_engine.undo_stack()
                        .execute(std::move(cmd));

                        m_selecting = false;
                    }
                    else
                    {
                        auto cmd =
                        std::make_unique<CmdSetNote>(
                            pat,
                            m_cursor_track,
                            m_cursor_row,
                            NOTE_EMPTY);

                        m_engine.undo_stack()
                        .execute(std::move(cmd));
                    }

                    redraw();
                    return 1;
                }

                if (key == FL_Up)
                {
                    if (shift && !m_selecting)
                    {
                        m_selecting = true;
                        m_sel_row_start = m_cursor_row;
                        m_sel_track_start = m_cursor_track;
                    }

                    m_cursor_row =
                    std::max(0, m_cursor_row - 1);

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

                if (key == FL_Down)
                {
                    if (shift && !m_selecting)
                    {
                        m_selecting = true;
                        m_sel_row_start = m_cursor_row;
                        m_sel_track_start = m_cursor_track;
                    }

                    m_cursor_row =
                    std::min(m_rows - 1,
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

                if (key == FL_Left)
                {
                    if (shift && !m_selecting)
                    {
                        m_selecting = true;
                        m_sel_row_start = m_cursor_row;
                        m_sel_track_start = m_cursor_track;
                    }

                    m_cursor_track =
                    std::max(0, m_cursor_track - 1);

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

                if (key == FL_Right)
                {
                    if (shift && !m_selecting)
                    {
                        m_selecting = true;
                        m_sel_row_start = m_cursor_row;
                        m_sel_track_start = m_cursor_track;
                    }

                    m_cursor_track =
                    std::min(m_tracks - 1,
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

                if (key == ' ')
                {
                    m_engine.toggle_play();
                    redraw();
                    return 1;
                }


                if (Fl::event_state(FL_CTRL))
                {
                    if (key == 'z')
                    {
                        m_engine.undo_stack().undo();
                        redraw();
                        return 1;
                    }

                    if (key == 'y')
                    {
                        m_engine.undo_stack().redo();
                        redraw();
                        return 1;
                    }

                    if (key == 'c')
                    {
                        if (!m_selecting) return 1;

                        auto& pat = m_engine.pattern();
                        auto& cb  = m_engine.clipboard();

                        int r0 = std::min(m_sel_row_start,
                                          m_sel_row_end);
                        int r1 = std::max(m_sel_row_start,
                                          m_sel_row_end);

                        int t0 = std::min(m_sel_track_start,
                                          m_sel_track_end);
                        int t1 = std::max(m_sel_track_start,
                                          m_sel_track_end);

                        cb.width  = t1 - t0 + 1;
                        cb.height = r1 - r0 + 1;
                        cb.notes.clear();

                        for (int t = t0; t <= t1; ++t)
                            for (int r = r0; r <= r1; ++r)
                                cb.notes.push_back(
                                    pat.track(t)
                                    .row(r).note);

                                return 1;
                    }
                    if (key == 'x')
                    {
                        if (!m_selecting) return 1;

                        auto& pat = m_engine.pattern();
                        auto& cb  = m_engine.clipboard();

                        // copy first (reuse Ctrl+C logic)
                        // then create CmdClearBlock

                        auto clear =
                        std::make_unique<CmdClearBlock>(
                            pat,
                            t0,t1,r0,r1);

                        std::vector<EditCommandPtr> group;
                        group.push_back(std::move(clear));

                        m_engine.undo_stack()
                        .execute_group(std::move(group));

                        m_selecting = false;
                        redraw();
                        return 1;
                    }

                    if (key == 'v')
                    {
                        auto& cb = m_engine.clipboard();
                        if (cb.width == 0) return 1;

                        auto& pat = m_engine.pattern();

                        auto cmd =
                        std::make_unique<CmdPasteBlock>(
                            pat,
                            cb,
                            m_cursor_track,
                            m_cursor_row);

                        m_engine.undo_stack()
                        .execute(std::move(cmd));

                        redraw();
                        return 1;
                    }


                }


                int note = key_to_note(key);
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

    void TrackerView::insert_note(uint8_t note)
    {
        auto& pat = m_engine.pattern();

        auto cmd =
        std::make_unique<CmdSetNote>(
            pat,
            m_cursor_track,
            m_cursor_row,
            note);

        m_engine.undo_stack()
        .execute(std::move(cmd));

        m_engine.preview_note(
            m_cursor_track,
            note);

        m_cursor_row++;
        redraw();
    }


}
