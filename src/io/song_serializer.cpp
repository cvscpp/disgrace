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

    bool SongSerializer::save(const Engine& engine, const ::std::string& path)
    {
        json j;
        j["tempo"] = engine.tempo();
        j["lpb"]   = engine.lpb();
        j["order"] = engine.order_list();
        j["patterns"] = json::array();

        for (size_t p = 0; p < engine.pattern_count(); ++p)
        {
            const Pattern& pat = engine.pattern(p);
            json jp;
            jp["rows"] = pat.row_count();
            jp["tracks"] = json::array();

            for (size_t t = 0; t < pat.track_count(); ++t)
            {
                json jtrack;
                jtrack["cols"] = pat.column_count(t);
                json jdata = json::array();

                for (size_t r = 0; r < pat.row_count(); ++r)
                {
                    json jrow = json::array();
                    for (size_t c = 0; c < pat.column_count(t); ++c) {
                        const TrackEvent& ev = pat.event(t, r, c);
                        json jev;
                        jev["note"]       = ev.note;
                        jev["sample"]     = ev.sample_idx;
                        jev["volume"]     = ev.volume;
                        jev["fx1"]        = ev.effect1;
                        jev["p1"]         = ev.param1;
                        jev["fx2"]        = ev.effect2;
                        jev["p2"]         = ev.param2;
                        jrow.push_back(jev);
                    }
                    jdata.push_back(jrow);
                }
                jtrack["data"] = jdata;
                jp["tracks"].push_back(jtrack);
            }
            j["patterns"].push_back(jp);
        }

        ::std::ofstream file(path);
        if (!file) return false;
        file << j.dump(2);
        return true;
    }

    bool SongSerializer::load(Engine& engine, const ::std::string& path)
    {
        ::std::ifstream file(path);
        if (!file) return false;

        json j;
        file >> j;

        engine.set_tempo(j["tempo"]);
        engine.set_lpb(j["lpb"]);
        engine.set_order(j["order"].get<::std::vector<uint8_t>>());

        auto& patterns = j["patterns"];
        for (size_t p = 0; p < patterns.size(); ++p)
        {
            if (p >= engine.pattern_count()) break;
            Pattern& pat = engine.pattern(p);
            size_t rows = patterns[p]["rows"];
            pat.resize_rows(rows);

            auto& jtracks = patterns[p]["tracks"];
            for (size_t t = 0; t < jtracks.size() && t < pat.track_count(); ++t)
            {
                size_t cols = jtracks[t]["cols"];
                pat.set_column_count(t, cols);
                auto& jdata = jtracks[t]["data"];
                
                for (size_t r = 0; r < jdata.size() && r < pat.row_count(); ++r)
                {
                    auto& jrow = jdata[r];
                    for (size_t c = 0; c < jrow.size() && c < pat.column_count(t); ++c) {
                        auto& jev = jrow[c];
                        auto& ev = pat.event(t, r, c);
                        ev.note       = jev["note"];
                        ev.sample_idx = jev["sample"];
                        ev.volume     = jev["volume"];
                        ev.effect1    = jev["fx1"];
                        ev.param1     = jev["p1"];
                        ev.effect2    = jev["fx2"];
                        ev.param2     = jev["p2"];
                    }
                }
            }
        }
        return true;
    }

} // namespace disgrace_ns
