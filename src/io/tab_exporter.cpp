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

#include "tab_exporter.h"
#include "../core/engine.h"
#include "../sequencer/pattern.h"
#include "../sequencer/note.h"
#include "../mixer/track.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace disgrace_ns {

namespace {
    const char* kNoteNames[] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};
    const size_t kSlotsPerLine = 16; // time slots per physical tab line (one bar)
    const char* kRestText = "--";    // silence
    const char* kHoldText = "- ";    // note-on is still ringing

    // "5-" for frets < 10, "12" for >= 10; unplayable is "x-".
    std::string slot_text(int fret) {
        if (fret < 0) return "x-";
        if (fret < 10) return std::to_string(fret) + "-";
        return std::to_string(fret);
    }
}

std::string TabExporter::note_name(int midi) {
    if (midi < 0 || midi > (int)NOTE_MAX) return "??";
    return std::string(kNoteNames[midi % 12]) + std::to_string(note_display_octave((uint8_t)midi));
}

std::string TabExporter::generate_tab(const Engine& engine, const TabExportConfig& config) {
    if (config.track >= engine.track_count() || config.open_strings.empty()) return "";

    const auto& track_obj = engine.track(config.track);
    auto order = engine.order_list();
    const size_t cols = config.open_strings.size();

    // Display order: highest-pitched string on the top tab line. Track
    // column c always writes to the line for open_strings[c].
    std::vector<size_t> line_of_column(cols);
    std::vector<size_t> display_order(cols);
    for (size_t i = 0; i < cols; ++i) display_order[i] = i;
    std::stable_sort(display_order.begin(), display_order.end(),
                     [&](size_t a, size_t b) { return config.open_strings[a] > config.open_strings[b]; });
    for (size_t line = 0; line < cols; ++line) line_of_column[display_order[line]] = line;

    std::stringstream ss;
    ss << engine.project_title() << "\n";
    if (!engine.project_artist().empty()) ss << engine.project_artist() << "\n";
    ss << "Track: " << (track_obj.name().empty() ? "Unnamed" : track_obj.name()) << "\n";
    ss << "Instrument: " << (config.is_bass ? "Bass" : "Guitar") << "\n";
    ss << "Tuning: ";
    for (size_t line = 0; line < cols; ++line) {
        if (line) ss << " ";
        ss << note_name(config.open_strings[display_order[line]]);
    }
    ss << "\n\n";

    // One physical block per pattern; each block holds kSlotsPerLine time
    // slots per string line. A row across all columns is one time slot;
    // aligned notes render as a chord. Notes ring until their column's
    // note-off, rendered as hold dashes. Trailing empty blocks of each
    // pattern are dropped.
    const uint8_t kNoteOff = 254;
    std::vector<int> col_string(cols, -1); // string currently sounding per column

    // Column c takes over string s; any other column ringing s releases it.
    auto claim = [&](size_t c, int s) {
        for (size_t o = 0; o < cols; ++o)
            if (o != c && col_string[o] == s) col_string[o] = -1;
        col_string[c] = s;
    };

    for (auto pat_idx : order) {
        const auto& pat = engine.pattern(pat_idx);
        const size_t rows = pat.row_count();
        const size_t pat_cols = std::min(pat.column_count(config.track), cols);

        // blocks[b][line][slot]
        std::vector<std::vector<std::vector<std::string>>> blocks;
        for (size_t block_start = 0; block_start < rows; block_start += kSlotsPerLine) {
            const size_t block_rows = std::min(kSlotsPerLine, rows - block_start);
            blocks.emplace_back(cols, std::vector<std::string>(block_rows, kRestText));

            for (size_t s = 0; s < block_rows; ++s) {
                const size_t row = block_start + s;
                auto& slots = blocks.back();

                // Note-offs first: they silence their column's ringing note.
                for (size_t c = 0; c < pat_cols; ++c)
                    if (pat.event(config.track, row, c).note == kNoteOff) col_string[c] = -1;

                std::vector<int> assigned(cols, -1); // string for this row's note-on, per column
                std::vector<bool> occupied(cols, false);
                auto is_note_on = [&](size_t c) {
                    const uint8_t n = pat.event(config.track, row, c).note;
                    return n < 128;
                };

                // Pass 1: every note that fits its own string claims it, so
                // fallbacks can never steal a string from a later column.
                for (size_t c = 0; c < pat_cols; ++c) {
                    if (!is_note_on(c)) continue;
                    const int note = pat.event(config.track, row, c).note;
                    const int own = note - config.open_strings[c];
                    if (own >= 0 && own <= kMaxFret) {
                        assigned[c] = (int)c;
                        occupied[c] = true;
                        claim(c, (int)c);
                    }
                }

                // Pass 2: out-of-range notes fall back to the lowest fret on
                // any free string, low strings win ties.
                for (size_t c = 0; c < pat_cols; ++c) {
                    if (!is_note_on(c) || assigned[c] >= 0) continue;
                    const int note = pat.event(config.track, row, c).note;
                    int target = -1, best_fret = kMaxFret + 1;
                    for (size_t o = 0; o < cols; ++o) {
                        if (occupied[o]) continue;
                        const int f = note - config.open_strings[o];
                        if (f < 0 || f > kMaxFret) continue;
                        if (f < best_fret ||
                            (f == best_fret && config.open_strings[o] < config.open_strings[target])) {
                            best_fret = f;
                            target = (int)o;
                        }
                    }
                    if (target >= 0) {
                        assigned[c] = target;
                        occupied[target] = true;
                        claim(c, target);
                    }
                }

                // Render note-ons and unplayable notes.
                for (size_t c = 0; c < pat_cols; ++c) {
                    if (!is_note_on(c)) continue;
                    if (assigned[c] >= 0) {
                        const int note = pat.event(config.track, row, c).note;
                        slots[line_of_column[assigned[c]]][s] =
                            slot_text(note - config.open_strings[assigned[c]]);
                    } else {
                        col_string[c] = -1;
                        slots[line_of_column[c]][s] = slot_text(-1); // x-
                    }
                }

                // Sustains: columns ringing a note without a new note-on
                // this row extend it with hold dashes.
                for (size_t c = 0; c < pat_cols; ++c) {
                    if (is_note_on(c) || col_string[c] < 0) continue;
                    auto& cell = slots[line_of_column[col_string[c]]][s];
                    if (cell == kRestText) cell = kHoldText;
                }
            }

            // Trim trailing all-rest blocks (a pattern longer than its
            // material should not print empty bars at the end).
            while (!blocks.empty()) {
                bool empty = true;
                for (const auto& line : blocks.back())
                    for (const auto& cell : line)
                        if (cell != kRestText) { empty = false; break; }
                if (!empty) break;
                blocks.pop_back();
            }
        }

        for (const auto& slots : blocks) {
            const size_t block_rows = slots.empty() ? 0 : slots[0].size();
            for (size_t line = 0; line < cols; ++line) {
                ss << note_name(config.open_strings[display_order[line]]) << "|";
                for (size_t s = 0; s < block_rows; ++s) ss << slots[line][s];
                ss << "|\n";
            }
            ss << "\n";
        }
    }

    return ss.str();
}

bool TabExporter::export_track(const Engine& engine, const TabExportConfig& config, const std::string& path) {
    const std::string tab = generate_tab(engine, config);
    if (tab.empty()) return false;
    std::ofstream f(path);
    if (!f.is_open()) return false;
    f << tab;
    return f.good();
}

} // namespace disgrace_ns
