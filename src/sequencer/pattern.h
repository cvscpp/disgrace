#pragma once

#include "note.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <nlohmann/json.hpp> // Add this line

namespace disgrace_ns
{

constexpr ::std::size_t MAX_TRACKS  = 64;
constexpr ::std::size_t MAX_ROWS    = 512;
constexpr ::std::size_t MAX_COLUMNS = 16;

constexpr uint8_t NOTE_EMPTY = 255;

class Pattern
{
public:
    Pattern(size_t rows = 64, size_t tracks = MAX_TRACKS) : rows(rows) {
        // Initialize m_data if necessary, but default constructor of std::array should handle it
    }
    ::std::size_t rows = 64;

    size_t track_count() const { return MAX_TRACKS; } // Add this line
    size_t row_count() const { return rows; }         // Add this line

    nlohmann::json to_json() const; // Add this line

    NoteEvent& event(::std::size_t track,
                     ::std::size_t row,
                     ::std::size_t column)
    {
        return m_data[track][row][column];
    }

    const NoteEvent& event(::std::size_t track,
                           ::std::size_t row,
                           ::std::size_t column) const // Note the 'const' here
    {
        return m_data[track][row][column];
    }

NoteEvent& event_for_track(::std::size_t current_track,
                           ::std::size_t row,
                           ::std::size_t column)
{
    return m_data[current_track][row][column];
}


private:
    using ColumnsArray = ::std::array<disgrace_ns::NoteEvent, MAX_COLUMNS>;
    using RowsArray    = ::std::array<ColumnsArray, MAX_ROWS>;
    using TracksArray  = ::std::array<RowsArray, MAX_TRACKS>;

    TracksArray m_data;

};

} // namespace disgrace_ns
