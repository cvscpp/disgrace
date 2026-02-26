#pragma once
#include "edit_command.h"
#include "../sequencer/pattern.h"
#include <vector>

namespace disgrace_ns
{

    class CmdClearBlock : public disgrace_ns::EditCommand
    {
    public:
        CmdClearBlock(disgrace_ns::Pattern& pat,
                      size_t track_start,
                      size_t track_end,
                      size_t row_start,
                      size_t row_end)
        : m_pattern(pat),
        m_t0(track_start),
        m_t1(track_end),
        m_r0(row_start),
        m_r1(row_end)
        {
            for (size_t t = m_t0; t <= m_t1; ++t)
            {
                for (size_t r = m_r0; r <= m_r1; ++r)
                {
                    m_old_values.push_back(
                        pat.event(t, r, 0).note); // Changed from pat.track(t).row(r).note
                }
            }
        }

        void apply() override
        {
            size_t idx = 0;

            for (size_t t = m_t0; t <= m_t1; ++t)
            {
                for (size_t r = m_r0; r <= m_r1; ++r)
                {
                    m_pattern.event(t, r, 0).note = // Changed from m_pattern.track(t).row(r).note
                    disgrace_ns::NOTE_EMPTY;
                    idx++;
                }
            }
        }

        void undo() override
        {
            size_t idx = 0;

            for (size_t t = m_t0; t <= m_t1; ++t)
            {
                for (size_t r = m_r0; r <= m_r1; ++r)
                {
                    m_pattern.event(t, r, 0).note = // Changed from m_pattern.track(t).row(r).note
                    m_old_values[idx++];
                }
            }
        }

    private:
        disgrace_ns::Pattern& m_pattern;

        size_t m_t0, m_t1;
        size_t m_r0, m_r1;

        ::std::vector<uint8_t> m_old_values;
    };

} // namespace disgrace_ns
