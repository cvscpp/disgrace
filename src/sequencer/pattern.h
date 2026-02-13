#pragma once

#include "note.h"
#include <array>

namespace dg
{

constexpr size_t MAX_TRACKS = 64;
constexpr size_t MAX_ROWS   = 512;
constexpr size_t MAX_COLUMNS = 16;

constexpr uint8_t NOTE_EMPTY = 255;


class Pattern
{
public:
    size_t rows = 64;

    NoteEvent& event(size_t track, size_t row, size_t column)
    {
        return m_data[track][row][column];
    }

private:
    std::array std::array std::array<NoteEvent, MAX_COLUMNS>,MAX_ROWS>,MAX_TRACKS> m_data();
};

}
