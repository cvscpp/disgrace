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
#include "instrument.h"
#include <map>
#include <vector>
#include <memory>

namespace disgrace_ns {

class Engine;

enum class TTSMode {
    RealTimeEspeak,  // Fast, ~100-200ms per phrase
    OfflineFestival  // Better quality, ~500ms-2s per phrase
};

class VoiceInstrument : public Instrument {
public:
    VoiceInstrument(Engine* engine = nullptr);

    void note_on(uint8_t note, uint8_t velocity, size_t column_index = 0, size_t offset_samples = 0, uint8_t sample_index = 0) override;
    void note_off(size_t column_index = 0) override;
    void panic() override;
    void set_volume(float vol) override;
    void set_pitch(float freq) override;
    void process(float* l, float* r, size_t nframes) override;

    // Voice-specific methods
    void set_text(const std::string& text, size_t column_index = 0);
    std::string get_text(size_t column_index = 0) const;
    
    void set_tts_mode(TTSMode mode) { m_tts_mode = mode; }
    TTSMode tts_mode() const { return m_tts_mode; }
    
    // Synthesis
    bool synthesize_text(const std::string& text, float base_freq);
    void clear_cache();
    size_t cache_size() const { return m_audio_cache.size(); }

protected:
    std::unique_ptr<Voice> create_voice() override { return nullptr; }

private:
    Engine* m_engine;
    TTSMode m_tts_mode = TTSMode::RealTimeEspeak;
    float m_volume = 1.0f;
    float m_current_pitch = 1.0f;
    
    // Text storage per column
    std::array<std::string, 16> m_column_text;
    
    // Audio cache: text → {samples_left, samples_right}
    struct CachedAudio {
        std::vector<float> left;
        std::vector<float> right;
    };
    std::map<std::string, CachedAudio> m_audio_cache;
    
    // Playback state
    std::string m_current_text;
    CachedAudio m_current_audio;
    size_t m_playback_pos = 0;
    bool m_playing = false;
    
    // TTS synthesis helpers
    bool synthesize_with_espeak(const std::string& text, std::vector<float>& out_l, std::vector<float>& out_r);
    bool synthesize_with_festival(const std::string& text, std::vector<float>& out_l, std::vector<float>& out_r);
    bool load_wav_from_file(const std::string& filepath, std::vector<float>& out_l, std::vector<float>& out_r);
};

} // namespace disgrace_ns
