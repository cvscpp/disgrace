#pragma once
#include "edit_command.h"
#include "../pattern/pattern.h"
#include "../engine/engine.h"
#include <vector>

namespace dg
{

    class CmdPasteBlock : public EditCommand
    {
    public:
        CmdPasteBlock(Pattern& pat,
                      const BlockClipboard& cb,
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
                            pat.track(track)
                            .row(row).note);
                    }
                    else
                    {
                        m_old_values.push_back(NOTE_EMPTY);
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
                        m_pattern.track(track)
                        .row(row).note =
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
                        m_pattern.track(track)
                        .row(row).note =
                        m_old_values[idx];
                    }

                    idx++;
                }
            }
        }

    private:
        Pattern& m_pattern;
        BlockClipboard m_clip;

        size_t m_dst_track;
        size_t m_dst_row;

        std::vector<uint8_t> m_old_values;
    };

}
