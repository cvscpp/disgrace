#pragma once
#include "edit_command.h"
#include "../pattern/pattern.h"

namespace dg
{

    class CmdSetNote : public EditCommand
    {
    public:
        CmdSetNote(Pattern& pat,
                   size_t track,
                   size_t row,
                   uint8_t new_note)
        : m_pattern(pat),
        m_track(track),
        m_row(row),
        m_new_note(new_note)
        {
            m_old_note =
            pat.track(track).row(row).note;
        }

        void apply() override
        {
            m_pattern.track(m_track)
            .row(m_row).note =
            m_new_note;
        }

        void undo() override
        {
            m_pattern.track(m_track)
            .row(m_row).note =
            m_old_note;
        }

    private:
        Pattern& m_pattern;
        size_t   m_track;
        size_t   m_row;

        uint8_t  m_old_note;
        uint8_t  m_new_note;
    };

}
