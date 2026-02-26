#include "pattern.h"
#include <nlohmann/json.hpp>

namespace disgrace_ns
{

// Helper function to serialize NoteEvent
void to_json(nlohmann::json& j, const NoteEvent& p) {
    j = nlohmann::json{
        {"note", p.note},
        {"instrument", p.instrument},
        {"volume", p.volume},
        {"effect", p.effect},
        {"param", p.param}
    };
}

nlohmann::json Pattern::to_json() const
{
    nlohmann::json j;
    j["rows"] = rows;

    nlohmann::json tracks_json = nlohmann::json::array();
    for (size_t track_idx = 0; track_idx < MAX_TRACKS; ++track_idx) {
        nlohmann::json rows_json = nlohmann::json::array();
        for (size_t row_idx = 0; row_idx < rows; ++row_idx) {
            nlohmann::json columns_json = nlohmann::json::array();
            for (size_t col_idx = 0; col_idx < MAX_COLUMNS; ++col_idx) {
                // Accessing private member m_data - this is fine since it's a member function
                columns_json.push_back(m_data[track_idx][row_idx][col_idx]);
            }
            rows_json.push_back(columns_json);
        }
        tracks_json.push_back(rows_json);
    }
    j["tracks"] = tracks_json;

    return j;
}

} // namespace disgrace_ns
