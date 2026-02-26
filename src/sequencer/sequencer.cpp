#include "sequencer.h"

namespace disgrace_ns
{

void disgrace_ns::Sequencer::set_timing(disgrace_ns::Timing* timing)
{
    m_timing = timing;
}

void disgrace_ns::Sequencer::set_order_list(const ::std::vector<int>& orders)
{
    m_order_list = orders;
}

void disgrace_ns::Sequencer::set_patterns(
    const ::std::vector<disgrace_ns::Pattern>& patterns_list)
{
    // Copy patterns from the list into the fixed-size array
    for (size_t i = 0; i < patterns_list.size() && i < MAX_PATTERNS; ++i)
    {
        m_patterns[i] = patterns_list[i];
    }
}

const disgrace_ns::NoteEvent& disgrace_ns::Sequencer::current_event(size_t track) const
{
    static disgrace_ns::NoteEvent empty;

    if (m_order_list.empty())
        return empty;

    int pattern_index =
    m_order_list[m_current_order];

    return m_patterns[pattern_index].event(track, m_current_row, 0);
}

void disgrace_ns::Sequencer::advance_row()
{
    if (m_order_list.empty())
        return;

    int pattern_index =
    m_order_list[m_current_order];

    m_current_row++;

    if (m_current_row >=
        m_patterns[pattern_index].rows)
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
} // End of advance_row

void disgrace_ns::Sequencer::jump_to_order(int order)
{
    m_pending_order_jump = order;
}

void disgrace_ns::Sequencer::break_to_row(int row)
{
    m_pending_row_break = row;
}

void disgrace_ns::Sequencer::pattern_break()
{
    m_current_row = 0;
    m_current_order++;
}

void disgrace_ns::Sequencer::jump_to(size_t order)
{
    if (order < m_order_length)
    {
        m_current_order = order;
        m_current_row = 0;
    }
}

} // namespace disgrace_ns
