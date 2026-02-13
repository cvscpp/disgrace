#pragma once

#include "pattern.h"
#include "timing.h"
#include <array>

namespace dg
{

constexpr size_t MAX_PATTERNS = 128;

class Sequencer
{
public:
    void set_timing(Timing* timing);

    void advance_row();
    void jump_to_order(int order);
    void break_to_row(int row);

    size_t current_order() const { return m_current_order; }
    size_t current_row() const { return m_current_row; }
    void set_order_list(const std::vector<int>& orders);
    void set_patterns(const std::vector<std::vector<std::vector<NoteEvent>>>& patterns);


    const NoteEvent& current_event(size_t track) const;

    static constexpr size_t MAX_ORDER = 256;

    std::array<uint8_t, MAX_ORDER> m_order{};
    size_t m_order_length = 1;

    size_t m_current_order = 0;

private:
    std::vector<int> m_order_list;

    // patterns[pattern][row][track]
    std::vector<std::vector<std::vector<NoteEvent>>> m_patterns;

    size_t m_current_order = 0;
    size_t m_current_row = 0;

    int m_pending_order_jump = -1;
    int m_pending_row_break = -1;
    Timing* m_timing = nullptr;

    std::array<Pattern, MAX_PATTERNS> m_patterns{};

    size_t m_current_pattern = 0;
    size_t m_current_row = 0;
};

}
