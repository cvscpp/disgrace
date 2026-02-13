#include "sequencer.h"

namespace dg
{

void Sequencer::set_timing(Timing* timing)
{
    m_timing = timing;
}

void Sequencer::set_order_list(const std::vector<int>& orders)
{
    m_order_list = orders;
}

void Sequencer::set_patterns(
    const std::vector<std::vector<std::vector<NoteEvent>>>& patterns)
{
    m_patterns = patterns;
}

const NoteEvent& Sequencer::current_event(size_t track) const
{
    static NoteEvent empty;

    if (m_order_list.empty())
        return empty;

    int pattern_index =
    m_order_list[m_current_order];

    return m_patterns[pattern_index]
    [m_current_row]
    [track];
}

void Sequencer::advance_row()
{
    if (m_order_list.empty())
        return;

    int pattern_index =
    m_order_list[m_current_order];

    m_current_row++;

    if (m_current_row >=
        m_patterns[pattern_index].size())
    {
        m_current_row = 0;
        m_current_order++;

        if (m_current_order >= m_order_list.size())
            m_current_order = 0;
    }

    // Apply pending jump
    if (m_pending_order_jump >= 0)
    {
        m_current_order = m_pending_order_jump;
        m_current_row = 0;
        m_pending_order_jump = -1;
    }

    if (m_pending_row_break >= 0)
    {
        m_current_row = m_pending_row_break;
        m_pending_row_break = -1;
    }
}

void Sequencer::jump_to_order(int order)
{
    m_pending_order_jump = order;
}

void Sequencer::break_to_row(int row)
{
    m_pending_row_break = row;
}

}

void Sequencer::pattern_break()
{
    m_current_row = 0;
    m_current_order++;
}

void Sequencer::jump_to(size_t order)
{
    if (order < m_order_length)
    {
        m_current_order = order;
        m_current_row = 0;
    }
}

const NoteEvent& Sequencer::current_event(size_t track) const
{
    return m_patterns[m_current_pattern]
        .event(track, m_current_row);
}

}

