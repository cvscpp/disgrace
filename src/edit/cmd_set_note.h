#pragma once
#include "edit_command.h"
#include "../sequencer/pattern.h"

namespace disgrace_ns
{

    class CmdSetNote : public disgrace_ns::EditCommand
    {
    public:
        CmdSetNote(disgrace_ns::Pattern& pat,
                   size_t track,
                   size_t row,
                   uint8_t new_note)
        : m_pattern(pat),
        m_track(track),
        m_row(row),
        m_new_note(new_note)
        {
            m_old_note =
            pat.event(track, row, 0).note; // Changed from pat.track(track).row(row).note
        }

        void apply() override
        {
            m_pattern.event(m_track, m_row, 0).note = // Changed from m_pattern.track(m_track).row(m_row).note
            m_new_note;
        }

        void undo() override
        {
            m_pattern.event(m_track, m_row, 0).note = // Changed from m_pattern.track(m_track).row(m_row).note
            m_old_note;
        }

    private:
        disgrace_ns::Pattern& m_pattern;
        size_t   m_track;
        size_t   m_row;

        uint8_t  m_old_note;
        uint8_t  m_new_note;
    };

} // namespace disgrace_ns
