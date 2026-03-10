#include "pattern.h"
#include <nlohmann/json.hpp>

namespace disgrace_ns
{

void to_json(nlohmann::json& j, const TrackEvent& p) {
    j = nlohmann::json{
        {"note", p.note},
        {"sample", p.sample_idx},
        {"volume", p.volume},
        {"fx1", p.effect1},
        {"p1", p.param1},
        {"fx2", p.effect2},
        {"p2", p.param2}
    };
}

nlohmann::json Pattern::to_json() const
{
    nlohmann::json j;
    j["rows"] = m_row_count;

    nlohmann::json tracks_json = nlohmann::json::array();
    for (size_t track_idx = 0; track_idx < m_tracks.size(); ++track_idx) {
        nlohmann::json track_info;
        track_info["cols"] = m_tracks[track_idx].columns;
        
        nlohmann::json rows_json = nlohmann::json::array();
        for (size_t row_idx = 0; row_idx < m_row_count; ++row_idx) {
            nlohmann::json columns_json = nlohmann::json::array();
            for (size_t col_idx = 0; col_idx < m_tracks[track_idx].columns; ++col_idx) {
                columns_json.push_back(m_tracks[track_idx].data[row_idx * MAX_COLS + col_idx]);
            }
            rows_json.push_back(columns_json);
        }
        track_info["data"] = rows_json;
        tracks_json.push_back(track_info);
    }
    j["tracks"] = tracks_json;

    return j;
}

} // namespace disgrace_ns
