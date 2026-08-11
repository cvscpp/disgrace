/*
 * Disgrace - Digital Audio Workstation
 * Copyright (C) 2025  Miroslav Shaltev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "midi_importer.h"
#include "../core/engine.h"
#include "../instrument/midi_instrument.h"
#include "../mixer/track.h"
#include "../sequencer/pattern.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <set>
#include <vector>

namespace disgrace_ns {

namespace {

constexpr size_t PATTERN_ROWS = MAX_ROWS; // 512
constexpr uint8_t NOTE_OFF = 254;
constexpr uint8_t MAX_TRACKER_NOTE = 119;

struct MidiNote {
    uint8_t channel = 0;
    uint8_t note = 0;
    uint8_t velocity = 64;
    uint64_t start_tick = 0;
    uint64_t end_tick = 0;
};

struct PlacedNote {
    uint8_t note = 0;
    uint8_t velocity = 64;
    uint64_t start_row = 0;
    uint64_t end_row = 0;
    uint8_t column = 0;
};

struct TempoSeg {
    uint64_t start_tick = 0;
    uint64_t end_tick = std::numeric_limits<uint64_t>::max();
    double us_per_quarter = 500000.0;
};

uint32_t read_be16(const std::vector<uint8_t>& d, size_t pos)
{
    if (pos + 2 > d.size()) return 0;
    return (uint32_t(d[pos]) << 8) | uint32_t(d[pos + 1]);
}

uint32_t read_be32(const std::vector<uint8_t>& d, size_t pos)
{
    if (pos + 4 > d.size()) return 0;
    return (uint32_t(d[pos]) << 24) | (uint32_t(d[pos + 1]) << 16) |
           (uint32_t(d[pos + 2]) << 8) | uint32_t(d[pos + 3]);
}

uint32_t read_vlq(const std::vector<uint8_t>& d, size_t& pos, size_t end)
{
    uint32_t v = 0;
    for (int k = 0; k < 4; ++k) {
        if (pos >= end) break;
        uint8_t b = d[pos++];
        v = (v << 7) | (b & 0x7F);
        if (!(b & 0x80)) break;
    }
    return v;
}

double tick_to_seconds(uint64_t tick, const std::vector<TempoSeg>& segs, double ppq)
{
    double t = 0.0;
    for (const auto& s : segs) {
        if (tick <= s.start_tick) break;
        uint64_t end = std::min<uint64_t>(tick, s.end_tick);
        if (end > s.start_tick) {
            t += double(end - s.start_tick) * s.us_per_quarter / (ppq * 1e6);
        }
        if (tick <= s.end_tick) break;
    }
    return t;
}

class SmfParser {
public:
    std::vector<MidiNote> notes;
    std::vector<std::pair<uint64_t, double>> tempos; // tick -> us per quarter
    std::map<uint8_t, uint8_t> programs;             // channel -> first program
    std::map<uint8_t, std::string> names;            // channel -> first track name
    uint32_t ppq = 480;

    bool parse(const std::vector<uint8_t>& data)
    {
        if (data.size() < 14 || std::memcmp(data.data(), "MThd", 4) != 0) return false;
        uint32_t hdr_len = read_be32(data, 4);
        if (hdr_len < 6 || 8 + hdr_len > data.size()) return false;
        uint16_t format = read_be16(data, 8);
        uint16_t ntrks = read_be16(data, 10);
        uint16_t division = read_be16(data, 12);
        if (division & 0x8000) return false; // SMPTE timing not supported
        ppq = division > 0 ? division : 480;
        (void)format;

        size_t pos = 8 + hdr_len;
        uint16_t parsed_tracks = 0;
        while (pos + 8 <= data.size() && parsed_tracks < ntrks) {
            if (std::memcmp(data.data() + pos, "MTrk", 4) != 0) {
                return false;
            }
            uint32_t trk_len = read_be32(data, pos + 4);
            size_t trk_end = pos + 8 + trk_len;
            if (trk_end > data.size()) {
                return false;
            }
            parse_track(data, pos + 8, trk_end);
            pos = trk_end;
            parsed_tracks++;
        }
        return !notes.empty();
    }

private:
    struct ActiveNote {
        uint64_t tick;
        uint8_t velocity;
    };

    void parse_track(const std::vector<uint8_t>& data, size_t begin, size_t end)
    {
        size_t pos = begin;
        uint8_t running = 0;
        uint64_t abs_tick = 0;
        std::string track_name;
        std::map<uint16_t, std::vector<ActiveNote>> active;
        std::set<uint8_t> track_channels;

        auto note_off = [&](uint8_t ch, uint8_t note, uint64_t tick) {
            uint16_t key = (uint16_t(ch) << 7) | note;
            auto it = active.find(key);
            if (it != active.end() && !it->second.empty()) {
                ActiveNote ev = it->second.back();
                it->second.pop_back();
                MidiNote mn;
                mn.channel = ch;
                mn.note = note;
                mn.velocity = ev.velocity;
                mn.start_tick = ev.tick;
                mn.end_tick = tick;
                notes.push_back(mn);
            }
            track_channels.insert(ch);
        };

        while (pos < end) {
            uint64_t dt = 0;
            for (int k = 0; k < 4; ++k) {
                if (pos >= end) break;
                uint8_t b = data[pos++];
                dt = (dt << 7) | (b & 0x7F);
                if (!(b & 0x80)) break;
            }
            abs_tick += dt;
            if (pos >= end) break;

            uint8_t b = data[pos];
            if (b == 0xFF) {
                pos++;
                if (pos >= end) break;
                uint8_t type = data[pos++];
                uint32_t len = read_vlq(data, pos, end);
                if (pos + len > end) return;
                if (type == 0x03 && len > 0 && track_name.empty()) {
                    track_name.assign(reinterpret_cast<const char*>(data.data() + pos), len);
                } else if (type == 0x51 && len >= 3) {
                    uint32_t us = (uint32_t(data[pos]) << 16) |
                                  (uint32_t(data[pos + 1]) << 8) | data[pos + 2];
                    tempos.push_back({abs_tick, double(us)});
                } else if (type == 0x2F) {
                    pos += len;
                    break; // end of track
                }
                pos += len;
            } else if (b == 0xF0 || b == 0xF7) {
                pos++;
                uint32_t len = read_vlq(data, pos, end);
                pos += len;
            } else {
                if (b >= 0x80) {
                    running = b;
                    pos++;
                }
                uint8_t status = running;
                uint8_t type = status & 0xF0;
                uint8_t ch = status & 0x0F;
                if (type == 0xC0 || type == 0xD0) {
                    if (pos >= end) break;
                    uint8_t d1 = data[pos++];
                    if (type == 0xC0 && !programs.count(ch)) programs[ch] = d1;
                } else if (type == 0x80 || type == 0x90 || type == 0xA0 ||
                           type == 0xB0 || type == 0xE0) {
                    if (pos + 2 > end) break;
                    uint8_t d1 = data[pos];
                    uint8_t d2 = data[pos + 1];
                    pos += 2;
                    if (type == 0x80 || (type == 0x90 && d2 == 0)) {
                        note_off(ch, d1, abs_tick);
                    } else if (type == 0x90) {
                        uint16_t key = (uint16_t(ch) << 7) | d1;
                        active[key].push_back({abs_tick, d2});
                        track_channels.insert(ch);
                    }
                } else {
                    return; // unknown status byte
                }
            }
        }

        // Flush unmatched note-ons; they ring to the end of the track.
        for (auto& kv : active) {
            for (const auto& ev : kv.second) {
                MidiNote mn;
                mn.channel = kv.first >> 7;
                mn.note = kv.first & 0x7F;
                mn.velocity = ev.velocity;
                mn.start_tick = ev.tick;
                mn.end_tick = abs_tick;
                notes.push_back(mn);
            }
        }

        if (!track_name.empty()) {
            for (uint8_t ch : track_channels) {
                if (!names.count(ch)) names[ch] = track_name;
            }
        }
    }
};

} // namespace

bool MidiImporter::import(Engine& engine, const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open MIDI file: " << path << std::endl;
        return false;
    }
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
    if (data.empty()) {
        std::cerr << "Empty MIDI file: " << path << std::endl;
        return false;
    }

    SmfParser parser;
    if (!parser.parse(data)) {
        std::cerr << "Failed to parse MIDI file: " << path << std::endl;
        return false;
    }

    // Target tempo: the file's first tempo event (or 120 BPM default).
    double bpm = 120.0;
    if (!parser.tempos.empty()) bpm = 60e6 / parser.tempos.front().second;
    bpm = std::max(10.0, std::min(400.0, bpm));
    const uint32_t lpb = 4;

    // Build the tempo map so note positions honour mid-song tempo changes.
    std::vector<TempoSeg> segs;
    segs.push_back({0, std::numeric_limits<uint64_t>::max(), 500000.0});
    for (const auto& t : parser.tempos) {
        bool merged = false;
        for (auto& s : segs) {
            if (s.start_tick == t.first) { s.us_per_quarter = t.second; merged = true; break; }
        }
        if (!merged) segs.push_back({t.first, std::numeric_limits<uint64_t>::max(), t.second});
    }
    std::sort(segs.begin(), segs.end(),
              [](const TempoSeg& a, const TempoSeg& b) { return a.start_tick < b.start_tick; });
    for (size_t i = 0; i + 1 < segs.size(); ++i) segs[i].end_tick = segs[i + 1].start_tick;

    // Convert raw notes to row positions and group them per channel.
    std::map<uint8_t, std::vector<PlacedNote>> by_channel;
    std::set<uint8_t> used_channels;
    uint64_t max_row_used = 0;
    for (const auto& n : parser.notes) {
        double start_sec = tick_to_seconds(n.start_tick, segs, parser.ppq);
        double end_sec = tick_to_seconds(n.end_tick, segs, parser.ppq);
        PlacedNote pn;
        pn.note = std::min<uint8_t>(n.note, MAX_TRACKER_NOTE);
        pn.velocity = std::max<uint8_t>(1, std::min<uint8_t>(127, n.velocity));
        pn.start_row = uint64_t(std::llround(start_sec * bpm * lpb / 60.0));
        pn.end_row = uint64_t(std::llround(end_sec * bpm * lpb / 60.0));
        by_channel[n.channel].push_back(pn);
        used_channels.insert(n.channel);
        max_row_used = std::max(max_row_used, pn.start_row);
        max_row_used = std::max(max_row_used, pn.end_row);
    }

    // Per-channel: assign notes to tracker columns to preserve polyphony.
    std::map<uint8_t, size_t> track_index;
    std::vector<std::pair<uint8_t, size_t>> channel_order; // channel, track index
    std::map<uint8_t, size_t> channel_max_cols;
    for (uint8_t ch : used_channels) {
        auto& list = by_channel[ch];
        std::sort(list.begin(), list.end(), [](const PlacedNote& a, const PlacedNote& b) {
            if (a.start_row != b.start_row) return a.start_row < b.start_row;
            return a.end_row < b.end_row;
        });
        std::array<uint64_t, MAX_COLS> busy{};
        size_t max_cols = 1;
        for (auto& pn : list) {
            size_t col = 0;
            while (col < MAX_COLS && busy[col] > pn.start_row) col++;
            if (col >= MAX_COLS) col = MAX_COLS - 1;
            pn.column = uint8_t(col);
            busy[col] = pn.end_row > pn.start_row ? pn.end_row : pn.start_row;
            max_cols = std::max(max_cols, col + 1);
        }
        size_t t = channel_order.size();
        channel_order.push_back({ch, t});
        channel_max_cols[ch] = max_cols;
    }

    if (channel_order.empty()) {
        std::cerr << "MIDI file contains no notes" << std::endl;
        return false;
    }

    // Replace the current project with the imported song.
    engine.m_tracks.clear();
    engine.m_buses.clear();
    engine.m_buses.emplace_back();
    engine.m_buses[0].set_name("Master");
    engine.m_instruments.clear();
    engine.clear_patterns();
    engine.m_order.clear();

    engine.set_tempo(bpm);
    engine.set_lpb(lpb);
    try {
        std::filesystem::path fp(path);
        engine.set_project_title(fp.stem().string());
    } catch (...) {}

    for (auto& [ch, t] : channel_order) {
        engine.add_track();
        track_index[ch] = t;

        auto midi = std::make_unique<MidiInstrument>(&engine);
        std::string name = "MIDI Ch " + std::to_string(ch + 1);
        auto it = parser.names.find(ch);
        if (it != parser.names.end() && !it->second.empty()) name = it->second;
        midi->set_name(name);
        midi->set_type(InstrumentType::Midi);
        midi->set_channel(ch);
        midi->set_program(parser.programs.count(ch) ? parser.programs[ch] : 0);

        engine.track(t).set_name(name);
        engine.track(t).set_instrument(midi.get());
        if (ch == 9) engine.track(t).set_notation(NotationType::Drums);
        engine.add_instrument(std::move(midi));
    }

    // Decide pattern layout (512 rows per pattern, clean beat-length loops).
    uint64_t needed = max_row_used + 1;
    if (needed < 64) needed = 64;
    needed = ((needed + lpb - 1) / lpb) * lpb;
    size_t num_patterns = size_t((needed + PATTERN_ROWS - 1) / PATTERN_ROWS);
    for (size_t i = 0; i < num_patterns; ++i) {
        size_t rows = size_t(std::min<uint64_t>(PATTERN_ROWS, needed - i * PATTERN_ROWS));
        engine.add_pattern(std::make_unique<Pattern>(rows, engine.track_count()));
        engine.m_order.push_back(i);
    }

    // Fill the patterns.
    for (auto& [ch, t] : channel_order) {
        Pattern& pat0 = engine.pattern(0);
        pat0.set_column_count(t, channel_max_cols[ch]);
        for (const auto& pn : by_channel[ch]) {
            size_t pat_idx = size_t(pn.start_row / PATTERN_ROWS);
            size_t row = size_t(pn.start_row % PATTERN_ROWS);
            TrackEvent& ev = engine.pattern(pat_idx).event(t, row, pn.column);
            ev.note = pn.note;
            ev.volume = pn.velocity;

            if (pn.end_row > pn.start_row) {
                size_t off_pat = size_t(pn.end_row / PATTERN_ROWS);
                size_t off_row = size_t(pn.end_row % PATTERN_ROWS);
                TrackEvent& off = engine.pattern(off_pat).event(t, off_row, pn.column);
                if (off.note == NOTE_EMPTY) off.note = NOTE_OFF;
            }
        }
    }

    engine.m_active_pattern.store(engine.m_order[0]);
    engine.m_order_start.store(0);
    engine.m_order_end.store(engine.m_order.size() - 1);
    engine.m_current_row = 0;
    engine.m_current_tick = 0;
    engine.m_edit_order_pos.store(0);
    engine.m_order_pos.store(0);

    std::cout << "MIDI import complete: " << engine.track_count() << " tracks, "
              << engine.pattern_count() << " patterns, " << parser.notes.size()
              << " notes, " << int(std::lround(bpm)) << " BPM" << std::endl;
    engine.mark_dirty();
    return true;
}

} // namespace disgrace_ns
