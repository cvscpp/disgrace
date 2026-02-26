#pragma once
#include "edit_command.h"
#include "../sequencer/pattern.h"
#include "../core/engine.h"
#include <vector>

namespace disgrace_ns
{

    class CmdPasteBlock : public disgrace_ns::EditCommand
    {
    public:
        CmdPasteBlock(disgrace_ns::Pattern& pat,
                      const disgrace_ns::BlockClipboard& cb,
                      size_t dst_track,
                      size_t dst_row)
        : m_pattern(pat),
        m_clip(cb),
        m_dst_track(dst_track),
        m_dst_row(dst_row)
        {
            for (size_t t = 0; t < cb.width; ++t)
            {
                for (size_t r = 0; r < cb.height; ++r)
                {
                    size_t track = dst_track + t;
                    size_t row   = dst_row + r;

                    if (track < pat.track_count() &&
                        row   < pat.row_count())
                    {
                        m_old_values.push_back(
                            pat.event(track, row, 0).note); // Changed from pat.track(t).row(r).note
                    }
                    else
                    {
                        m_old_values.push_back(disgrace_ns::NOTE_EMPTY);
                    }
                }
            }
        }

        void apply() override
        {
            size_t idx = 0;

            for (size_t t = 0; t < m_clip.width; ++t)
            {
                for (size_t r = 0; r < m_clip.height; ++r)
                {
                    size_t track = m_dst_track + t;
                    size_t row   = m_dst_row + r;

                    if (track < m_pattern.track_count() &&
                        row   < m_pattern.row_count())
                    {
                        m_pattern.event(track, row, 0).note = // Changed from m_pattern.track(track).row(row).note
                        m_clip.notes[idx];
                    }

                    idx++;
                }
            }
        }

        void undo() override
        {
            size_t idx = 0;

            for (size_t t = 0; t < m_clip.width; ++t)
            {
                for (size_t r = 0; r < m_clip.height; ++r)
                {
                    size_t track = m_dst_track + t;
                    size_t row   = m_dst_row + r;

                    if (track < m_pattern.track_count() &&
                        row   < m_pattern.row_count())
                    {
                        m_pattern.event(track, row, 0).note = // Changed from m_pattern.track(track).row(row).note
                        m_old_values[idx];
                    }

                    idx++;
                }
            }
        }

    private:
        disgrace_ns::Pattern& m_pattern;
        disgrace_ns::BlockClipboard m_clip;

        size_t m_dst_track;
        size_t m_dst_row;

        ::std::vector<uint8_t> m_old_values;
    };

} // namespace disgrace_ns
