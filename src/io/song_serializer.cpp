#include "song_serializer.h"
#include "../core/engine.h"
#include "../sequencer/sequencer.h"
#include "../sequencer/pattern.h"
#include "../mixer/track.h"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace disgrace_ns
{

    bool disgrace_ns::SongSerializer::save(const disgrace_ns::Engine& engine,
                              const ::std::string& path)
    {
        json j;

        // Timing
        j["tempo"] = engine.tempo();
        j["lpb"]   = engine.lpb();

        // Order
        j["order"] = engine.order_list();

        // Patterns
        j["patterns"] = json::array();

        for (size_t p = 0; p < engine.pattern_count(); ++p)
        {
            json jp;
            const disgrace_ns::Pattern& pat = engine.pattern(p);

            jp["rows"] = pat.rows;

            jp["tracks"] = json::array();

            for (size_t t = 0; t < MAX_TRACKS; ++t)
            {
                json jtrack = json::array();

                for (size_t r = 0; r < pat.rows; ++r)
                {
                    const disgrace_ns::NoteEvent& ev =
                    pat.event(t, r, 0); // Corrected to 3 arguments

                    json jev;
                    jev["note"]       = ev.note;
                    jev["instrument"] = ev.instrument;
                    jev["effect"]     = static_cast<int>(ev.effect);
                    jev["param"]      = ev.param;     // Corrected member name


                    jtrack.push_back(jev);
                }

                jp["tracks"].push_back(jtrack);
            }

            j["patterns"].push_back(jp);
        }

        ::std::ofstream file(path);
        if (!file)
            return false;

        file << j.dump(2);
        return true;
    }

    bool disgrace_ns::SongSerializer::load(disgrace_ns::Engine& engine,
                              const ::std::string& path)
    {
        ::std::ifstream file(path);
        if (!file)
            return false;

        json j;
        file >> j;

        engine.set_tempo(j["tempo"]);
        engine.set_lpb(j["lpb"]);

        engine.set_order(j["order"].get<::std::vector<uint8_t>>());

        auto& patterns = j["patterns"];

        for (size_t p = 0; p < patterns.size(); ++p)
        {
            disgrace_ns::Pattern& pat = engine.pattern(p);

            pat.rows = patterns[p]["rows"];

            for (size_t t = 0; t < MAX_TRACKS; ++t)
            {
                for (size_t r = 0; r < pat.rows; ++r)
                {
                    auto& jev =
                    patterns[p]["tracks"][t][r];

                    auto& ev = pat.event(t, r, 0); // Corrected to 3 arguments

                    ev.note       = jev["note"];
                    ev.instrument = jev["instrument"];
                    ev.effect     = static_cast<uint8_t>(jev["effect"]);
                    ev.param =
                    jev["param"]; // Corrected member name

                }
            }
        }

        return true;
    }

} // namespace disgrace_ns
