#pragma once

#include <memory>

#include "../sequencer/timing.h"
#include "../mixer/track.h"
#include <vector>

#include "../audio/sample.h"

#include "../edit/undo_stack.h"

#include "midi/MidiQueue.h"
#include "midi/MidiInput.h"

#include "metronome.h"

#include "MasterBus.h"

namespace dg
{

    struct Instrument
    {
        std::string name;
        std::shared_ptr<Sample> sample;
    };

    struct BlockClipboard
    {
        size_t width = 0;
        size_t height = 0;

        std::vector<uint8_t> notes;
    };



class Transport;

class Engine
{
public:
    Engine();
    ~Engine();

    bool initialize();
    void shutdown();

    void start();
    void stop();

    Transport& transport();

    void process_audio(float* out_l,
                       float* out_r,
                       size_t nframes);

    double tempo() const;
    uint32_t lpb() const;

    void set_tempo(double);
    void set_lpb(uint32_t);

    const Pattern& pattern(size_t) const;
    Pattern& pattern(size_t);

    size_t pattern_count() const;

    std::vector<uint8_t> order_list() const;
    void set_order(const std::vector<uint8_t>&);

    size_t m_samples_until_next_tick = 0;
    int    m_current_tick = 0;

    void handle_effect_row_start(size_t track_index,
                                         const NoteEvent& ev)

    size_t Engine::current_row() const;
    bool   Engine::is_playing() const;
    void set_active_pattern(size_t index);
    size_t active_pattern() const;
    void preview_note(size_t track, uint8_t note);

    Pattern& pattern(); // current active pattern

    std::vector<Instrument> m_instruments;

    Instrument& instrument(size_t index);
    size_t instrument_count() const;
    std::vector<size_t> m_order;

    void add_order(size_t pattern);
    size_t current_order_pos() const;
    UndoStack& undo_stack();
    BlockClipboard& clipboard();

    MidiQueue<MidiMessage, 1024> m_midi_queue;
    MidiInput m_midi;

    std::atomic<bool> m_record_enabled{false};
    size_t m_record_track{0};

    void enable_record(bool e);
    void set_record_track(size_t t);
    void record_note(uint8_t note);

    Metronome m_metronome;

    size_t m_samples_until_next_beat{0};
    bool m_metronome_enabled{true};

    double tempo() const;

    void set_tempo(double bpm);
    MasterBus m_master;

    void set_master_gain(float g);
    float master_gain() const;
    float master_meter() const;
    bool render_to_wav(const std::string& path);
    void process_block(float* l,
                       float* r,
                       size_t nframes);

private:
    UndoStack m_undo;
    bool m_initialized;
    BlockClipboard m_clipboard;

    std::unique_ptr<Transport> m_transport;

    Timing m_timing;

    std::vector<Track> m_tracks;

    static constexpr size_t MAX_BLOCK = 2048;

    float m_mix_l[MAX_BLOCK];
    float m_mix_r[MAX_BLOCK];

    float m_track_l[MAX_TRACKS][MAX_BLOCK];
    float m_track_r[MAX_TRACKS][MAX_BLOCK];

    RingBuffer<EngineCommand, 128> m_cmd_queue;

    std::vector<Pattern> m_patterns;
    std::atomic<size_t>  m_active_pattern{0};


    bool write_wav(
        const std::string& path,
        const std::vector<float>& l,
        const std::vector<float>& r,
        size_t frames)

    //TODO where to put this?
    //m_current_row.store(m_sequencer.current_row(), std::memory_order_relaxed);

    // Future:
    // std::unique_ptr<AudioBackend> m_audio;
    // std::unique_ptr<Sequencer> m_sequencer;
    // std::unique_ptr<Mixer> m_mixer;
};

} // namespace dg
