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

#pragma once

#include <imgui.h>
#include <vector>
#include <string>
#include "../sequencer/pattern.h"

namespace disgrace_ns
{
    class Engine;

    class TrackerWidget
    {
    public:
        TrackerWidget(Engine& engine);
        ~TrackerWidget() = default;

        void render();
        void handleKeyEvent(int key, int mods);

        void setCursorRow(int row) { m_cursorRow = row; clampCursor(); }
        void setCursorTrack(int track) { m_cursorTrack = track; clampCursor(); }
        int cursorRow() const { return m_cursorRow; }
        int cursorTrack() const { return m_cursorTrack; }
        int cursorCol() const { return m_cursorCol; }
        int cursorField() const { return m_cursorField; }

    private:
        void clampCursor();
        void deleteCurrentField();
        void insertNoteFromAction(int key, int mods);
        void insertNote(uint8_t note);

        Engine& m_engine;

        struct TrackUI {
            float x, w;
        };
        std::vector<TrackUI> m_trackUI;

        int m_cursorRow = 0;
        int m_cursorTrack = 0;
        int m_cursorCol = 0;
        int m_cursorField = 0;

        bool m_selActive = false;
        int m_selStartTrack = -1;
        int m_selStartRow = -1;
        int m_selEndTrack = -1;
        int m_selEndRow = -1;

        float m_rowHeight = 18.0f;
        float m_charWidth = 8.0f;
        float m_headerHeight = 20.0f;
        float m_scrollX = 0.0f;
        float m_scrollY = 0.0f;
    };
}
