#include "engine.h"
#include "transport.h"
#include "../audio/jack_backend.h"
#include "../instrument/sample_instrument.h"
#include "../instrument/soundfont_instrument.h"

#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <vector>
#include <fstream>
#include <cstdint>

namespace disgrace_ns
{

bool Engine::render_to_wav(const ::std::string& path)
{
    size_t total_samples = total_song_samples();
    ::std::vector<float> left(total_samples);
    ::std::vector<float> right(total_samples);

    reset_transport_to_start();

    size_t rendered = 0;
    const size_t block_size = 512;

    while (rendered < total_samples)
    {
        size_t block = ::std::min(block_size, total_samples - rendered);
        process_block(&left[rendered], &right[rendered], block);
        rendered += block;
    }

    return write_wav(path, left, right, total_samples);
}

bool Engine::write_wav(const ::std::string& path, const ::std::vector<float>& l, const ::std::vector<float>& r, size_t frames)
{
    ::std::ofstream file(path, ::std::ios::binary);
    if (!file.is_open()) return false;

    uint32_t sample_rate = m_sample_rate;
    uint16_t bits = 16;
    uint16_t channels = 2;
    uint32_t byte_rate = sample_rate * channels * bits/8;
    uint16_t block_align = channels * bits/8;
    uint32_t data_size = frames * block_align;

    file.write("RIFF", 4);
    uint32_t chunk_size = 36 + data_size;
    file.write(reinterpret_cast<char*>(&chunk_size), 4);
    file.write("WAVE", 4);
    file.write("fmt ", 4);
    uint32_t subchunk1_size = 16;
    uint16_t audio_format = 1;
    file.write(reinterpret_cast<char*>(&subchunk1_size), 4);
    file.write(reinterpret_cast<char*>(&audio_format), 2);
    file.write(reinterpret_cast<char*>(&channels), 2);
    file.write(reinterpret_cast<char*>(&sample_rate), 4);
    file.write(reinterpret_cast<char*>(&byte_rate), 4);
    file.write(reinterpret_cast<char*>(&block_align), 2);
    file.write(reinterpret_cast<char*>(&bits), 2);
    file.write("data", 4);
    file.write(reinterpret_cast<char*>(&data_size), 4);

    for (size_t i = 0; i < frames; ++i)
    {
        float sl = l[i];
        float sr = r[i];
        sl = ::std::max(-1.f, ::std::min(1.f, sl));
        sr = ::std::max(-1.f, ::std::min(1.f, sr));
        int16_t il = static_cast<int16_t>(sl * 32767.f);
        int16_t ir = static_cast<int16_t>(sr * 32767.f);
        file.write(reinterpret_cast<char*>(&il), 2);
        file.write(reinterpret_cast<char*>(&ir), 2);
    }
    return true;
}

Engine::Engine() : m_initialized(false)
{
    m_patterns.emplace_back(64, 8);
    m_order.push_back(0);
    m_midi.set_callback([this](const MidiMessage& msg) { m_midi_queue.push(msg); });
    m_midi.start();
    m_metronome.set_sample_rate(m_sample_rate);
    m_samples_until_next_beat = m_timing.samples_per_beat();
}

Engine::~Engine() { if (m_initialized) shutdown(); }

bool Engine::initialize()
{
    if (m_initialized) return true;
    m_transport = ::std::make_unique<Transport>();
    
    m_backend = ::std::make_unique<JackBackend>(this, m_num_ins, m_num_outs, m_num_midi_ins, m_num_midi_outs);
    if (!m_backend->start()) {
        fprintf(stderr, "Warning: Could not connect to JACK. Audio will be disabled.\n");
    }

    m_initialized = true;
    return true;
}

void Engine::reinitialize_audio(uint32_t num_ins, uint32_t num_outs, uint32_t num_midi_ins, uint32_t num_midi_outs)
{
    m_num_ins = num_ins;
    m_num_outs = num_outs;
    m_num_midi_ins = num_midi_ins;
    m_num_midi_outs = num_midi_outs;
    if (m_backend) { m_backend->stop(); m_backend.reset(); }
    m_backend = ::std::make_unique<JackBackend>(this, m_num_ins, m_num_outs, m_num_midi_ins, m_num_midi_outs);
    m_backend->start();
}

bool Engine::audio_active() const
{
    return m_backend && m_backend->is_active();
}

void Engine::new_project()
{
    m_patterns.clear();
    m_instruments.clear();
    m_order.clear();
    m_tracks.clear();
    m_active_pattern = 0;
    m_current_row = 0;
    reset_transport_to_start();
}

void Engine::shutdown()
{
    if (!m_initialized) return;
    stop();
    m_transport.reset();
    m_initialized = false;
}

void Engine::start() { if (m_initialized) m_transport->play(); }
void Engine::stop() { if (m_initialized) m_transport->stop(); }
void Engine::enable_record(bool e) { m_record_enabled.store(e); }
void Engine::set_record_track(size_t t) { m_record_track = t; }
void Engine::preview_note(size_t track, uint8_t note) { if (track < m_tracks.size()) m_tracks[track].note_on(note, 100); }
Instrument& Engine::instrument(size_t i) { return *m_instruments[i]; }
size_t Engine::instrument_count() const { return m_instruments.size(); }

int Engine::get_instrument_index(Instrument* inst) const
{
    if (!inst) return -1;
    for (size_t i = 0; i < m_instruments.size(); ++i) {
        if (m_instruments[i].get() == inst) return (int)i;
    }
    return -1;
}

void Engine::add_instrument()
{
    m_instruments.push_back(std::make_unique<NoneInstrument>());
    m_instruments.back()->set_name("Instrument " + std::to_string(m_instruments.size()));
}

void Engine::remove_instrument(size_t index)
{
    if (index < m_instruments.size()) {
        // We should also check if any track is using this instrument
        for (auto& track : m_tracks) {
            if (track.instrument() == m_instruments[index].get()) {
                track.set_instrument(nullptr);
            }
        }
        m_instruments.erase(m_instruments.begin() + index);
    }
}

void Engine::set_instrument_type(size_t index, InstrumentType type)
{
    if (index >= m_instruments.size()) return;
    if (m_instruments[index]->type() == type) return;

    std::string name = m_instruments[index]->name();
    std::unique_ptr<Instrument> new_inst;

    switch (type) {
        case InstrumentType::Sampler:
            new_inst = std::make_unique<SampleInstrument>((double)m_sample_rate);
            break;
        case InstrumentType::SoundFont:
            new_inst = std::make_unique<SoundFontInstrument>((double)m_sample_rate);
            break;
        default:
            new_inst = std::make_unique<NoneInstrument>();
            break;
    }

    new_inst->set_name(name);
    new_inst->set_type(type);

    // Update tracks that use this instrument
    for (auto& track : m_tracks) {
        if (track.instrument() == m_instruments[index].get()) {
            track.set_instrument(new_inst.get());
        }
    }

    m_instruments[index] = std::move(new_inst);
}
size_t Engine::max_track_latency() const { size_t max = 0; for (const auto& t : m_tracks) max = ::std::max(max, t.total_latency()); return max; }
Transport& Engine::transport() { return *m_transport; }

void Engine::record_note(uint8_t note)
{
    size_t row = current_row();
    Pattern& current_pattern = pattern();
    current_pattern.event(m_record_track, row, 0).note = note;
}

void Engine::record() { transport().record(); }
double Engine::tempo() const { return m_timing.tempo(); }
void Engine::set_tempo(double bpm) { EngineCommand cmd; cmd.type = EngineCommandType::SetTempo; cmd.value = bpm; m_cmd_queue.push(cmd); }
void Engine::set_master_gain(float g) { m_master.set_gain(g); }
float Engine::master_gain() const { return m_master.gain(); }
float Engine::master_meter() const { return m_master.meter(); }

void Engine::process_audio(float* out_l, float* out_r, size_t nframes)
{
    EngineCommand cmd;
    if (m_metronome_enabled && transport().state() != TransportState::Stopped)
    {
        m_metronome.process(m_mix_l, m_mix_r, nframes, m_samples_until_next_beat, m_timing.samples_per_beat());
    }

    MidiMessage msg;
    while (m_midi_queue.pop(msg))
    {
        uint8_t status = msg.status & 0xF0;
        if (status == 0x90 && msg.data2 > 0)
        {
            uint8_t note = msg.data1;
            uint8_t vel  = msg.data2;
            m_tracks[m_record_track].note_on(note, vel);
            if (m_record_enabled && transport().state() == TransportState::Recording) record_note(note);
        }
    }

    while (m_cmd_queue.pop(cmd))
    {
        switch (cmd.type)
        {
            case EngineCommandType::Play: m_playing.store(true); break;
            case EngineCommandType::Stop: m_playing.store(false); break;
            case EngineCommandType::SetTempo: m_timing.set_lpb(static_cast<uint32_t>(cmd.value)); break;
        }
    }

    if (transport().state() != TransportState::Playing && transport().state() != TransportState::Recording)
    {
        ::std::fill(out_l, out_l + nframes, 0.f);
        ::std::fill(out_r, out_r + nframes, 0.f);
        return;
    }
    process_block(out_l, out_r, nframes);
}

size_t Engine::total_song_samples() const
{
    size_t total_rows = 0;
    for (auto p : m_patterns) total_rows += p.rows;
    return total_rows * m_timing.samples_per_row();
}

void Engine::process_block(float* l, float* r, size_t nframes)
{
    size_t processed = 0;
    while (processed < nframes)
    {
        if (m_samples_until_next_tick == 0)
        {
            process_tick();
            m_samples_until_next_tick = m_timing.samples_per_tick();
        }
        size_t block = ::std::min(m_samples_until_next_tick, nframes - processed);
        render_block(l + processed, r + processed, block);
        processed += block;
        m_samples_until_next_tick -= block;
    }
    m_master.process(m_mix_l, m_mix_r, nframes);
    for (size_t i = 0; i < nframes; ++i) { l[i] = m_mix_l[i]; r[i] = m_mix_r[i]; }
}

void Engine::set_active_pattern(size_t index) { if (index < m_patterns.size()) m_active_pattern.store(index); }
size_t Engine::active_pattern() const { return m_active_pattern.load(); }
Pattern& Engine::pattern() { return m_patterns[m_active_pattern.load()]; }

inline float Engine::soft_clip(float x) { const float limit = 0.95f; if (x > limit) return limit; if (x < -limit) return -limit; return x; }

void Engine::render_block(float* out_l, float* out_r, size_t frames)
{
    for (size_t i = 0; i < frames; ++i) { m_mix_l[i] = 0.f; m_mix_r[i] = 0.f; }
    for (size_t t = 0; t < m_tracks.size(); ++t)
    {
        auto& track = m_tracks[t];
        track.process(m_track_l[t], m_track_r[t], frames);
        for (size_t i = 0; i < frames; ++i) { m_mix_l[i] += m_track_l[t][i]; m_mix_r[i] += m_track_r[t][i]; }
    }
    for (size_t i = 0; i < frames; ++i) { out_l[i] = soft_clip(m_mix_l[i] * master_gain()); out_r[i] = soft_clip(m_mix_r[i] * master_gain()); }
}

void Engine::handle_effect_row_start(size_t t, const NoteEvent& ev)
{
    auto& track = m_tracks[t];
    uint8_t fx = ev.effect;
    uint8_t param = ev.param;
    switch (fx)
    {
        case 0x0F: if (param < 32) m_timing.set_speed(param); else m_timing.set_lpb(param); break;
        case 0x0C: track.set_volume(param / 64.f); break;
        case 0x0A: track.m_fx_state.vol_slide_up = (param >> 4); track.m_fx_state.vol_slide_down = (param & 0x0F); break;
        case 0x03: track.m_fx_state.porta_speed = param * 0.0005f; track.m_fx_state.porta_active = true; break;
        case 0x0E: { uint8_t sub = param >> 4; uint8_t val = param & 0x0F; if (sub == 0x9) { track.m_fx_state.retrig_ticks = val; track.m_fx_state.retrig_counter = 0; } else if (sub == 0xC) track.m_fx_state.note_cut_tick = val; break; }
    }
}

void Engine::handle_effect(const NoteEvent& ev)
{
    switch (static_cast<int>(ev.effect))
    {
        case static_cast<int>(EffectType::SetTempo): m_timing.set_bpm(ev.param); break;
    }
}

void Engine::handle_midi(uint8_t* data, size_t size)
{
    if (size < 3) return;
    uint8_t status = data[0] & 0xF0;
    if (status == 0x90 && data[2] > 0) m_tracks[0].note_on(data[1], data[2]);
    else if (status == 0x80 || (status == 0x90 && data[2] == 0)) m_tracks[0].note_off();
}

UndoStack& Engine::undo_stack() { return m_undo; }
BlockClipboard& Engine::clipboard() { return m_clipboard; }

void Engine::toggle_play() { if (transport().state() == TransportState::Playing) stop(); else start(); }
void Engine::set_loop(bool enable) { m_loop_pattern.store(enable); }
void Engine::set_play_position(size_t pattern, size_t row) { if (pattern < m_patterns.size()) m_active_pattern.store(pattern); m_current_row = row; m_samples_until_next_tick = m_timing.samples_per_row(); }
void Engine::set_metronome_enabled(bool enabled) { m_metronome_enabled.store(enabled); }
TransportState Engine::transport_state() const { return m_transport->state(); }

size_t Engine::add_pattern_to_order() { m_patterns.emplace_back(); size_t new_pattern_index = m_patterns.size() - 1; m_order.push_back(new_pattern_index); return new_pattern_index; }
void Engine::remove_pattern_from_order(size_t order_pos) { if (order_pos < m_order.size()) m_order.erase(m_order.begin() + order_pos); }
size_t Engine::copy_pattern_in_order(size_t order_pos) { if (order_pos < m_order.size()) { size_t pattern_index_to_copy = m_order[order_pos]; if (pattern_index_to_copy < m_patterns.size()) { m_patterns.push_back(m_patterns[pattern_index_to_copy]); size_t new_pattern_index = m_patterns.size() - 1; m_order.insert(m_order.begin() + order_pos + 1, new_pattern_index); return new_pattern_index; } } return static_cast<size_t>(-1); }

void Engine::play() { start(); }
size_t Engine::current_row() const { return m_current_row; }
bool Engine::is_playing() const { return m_playing.load(); }
void Engine::set_lpb(uint32_t lpb) { m_timing.set_lpb(lpb); }
uint32_t Engine::lpb() const { return m_timing.lpb(); }
const Pattern& Engine::pattern(size_t index) const { return m_patterns[index]; }
Pattern& Engine::pattern(size_t index) { return m_patterns[index]; }
size_t Engine::pattern_count() const { return m_patterns.size(); }

::std::vector<uint8_t> Engine::order_list() const { ::std::vector<uint8_t> list; for (size_t p_idx : m_order) list.push_back(static_cast<uint8_t>(p_idx)); return list; }
void Engine::set_order(const ::std::vector<uint8_t>& order_list) { m_order.clear(); for (uint8_t p_idx : order_list) m_order.push_back(static_cast<size_t>(p_idx)); }
void Engine::set_current_instrument(size_t index) {}
void Engine::toggle_metronome() { m_metronome_enabled.store(!m_metronome_enabled.load()); }

void Engine::reset_transport_to_start() { m_current_row = 0; m_current_tick = 0; m_samples_until_next_tick = 0; m_metronome.reset(); }
void Engine::process_tick() { m_current_row = (m_current_row + 1) % m_patterns[m_active_pattern.load()].rows; }
Track& Engine::track(size_t index) { if (index >= m_tracks.size()) throw std::out_of_range("Track index out of bounds"); return m_tracks[index]; }
size_t Engine::track_count() const { return m_tracks.size(); }

void Engine::add_track()
{
    m_tracks.emplace_back();
    m_tracks.back().set_name("Track " + std::to_string(m_tracks.size()));
}

void Engine::remove_track(size_t index)
{
    if (index < m_tracks.size()) {
        m_tracks.erase(m_tracks.begin() + index);
    }
}

void Engine::move_track(size_t from, size_t to)
{
    if (from < m_tracks.size() && to < m_tracks.size() && from != to) {
        auto it_from = m_tracks.begin() + from;
        auto it_to = m_tracks.begin() + to;
        
        // Move element
        Track t = std::move(*it_from);
        m_tracks.erase(it_from);
        m_tracks.insert(m_tracks.begin() + to, std::move(t));
    }
}

} // namespace disgrace_ns
