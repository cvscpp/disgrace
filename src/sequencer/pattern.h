#pragma once

#include "note.h"
#include <vector>
#include <cstddef>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace disgrace_ns
{

constexpr uint8_t NOTE_EMPTY = 255;

struct TrackData {
    size_t columns = 1;
    // data[row][column]
    std::vector<std::vector<TrackEvent>> rows;
    
    TrackData(size_t row_count = 64, size_t col_count = 1) : columns(col_count) {
        rows.resize(row_count);
        for(auto& r : rows) r.resize(columns);
    }
};

class Pattern
{
public:
    Pattern(size_t rows = 64, size_t tracks = 8) : m_row_count(rows) {
        m_tracks.resize(tracks);
        for(auto& t : m_tracks) {
            t.rows.resize(rows);
            for(auto& r : t.rows) r.resize(t.columns);
        }
    }

    size_t track_count() const { return m_tracks.size(); }
    size_t row_count() const { return m_row_count; }
    
    size_t column_count(size_t track) const { 
        if (track >= m_tracks.size()) return 0;
        return m_tracks[track].columns; 
    }

    void set_column_count(size_t track, size_t cols) {
        if (track >= m_tracks.size()) return;
        m_tracks[track].columns = cols;
        for(auto& r : m_tracks[track].rows) r.resize(cols);
    }

    nlohmann::json to_json() const;

    TrackEvent& event(size_t track, size_t row, size_t column) {
        return m_tracks[track].rows[row][column];
    }

    const TrackEvent& event(size_t track, size_t row, size_t column) const {
        return m_tracks[track].rows[row][column];
    }

    void resize_rows(size_t new_rows) {
        m_row_count = new_rows;
        for(auto& t : m_tracks) {
            t.rows.resize(new_rows);
            for(auto& r : t.rows) r.resize(t.columns);
        }
    }

    void resize_tracks(size_t new_tracks) {
        m_tracks.resize(new_tracks, TrackData(m_row_count, 1));
    }

private:
    size_t m_row_count;
    std::vector<TrackData> m_tracks;
};

} // namespace disgrace_ns
