#include "engine.h"
#include "transport.h"

#include <stdexcept>
#include <cstring>
#include <algorithm>

namespace dg
{

Engine::Engine()
    : m_initialized(false)
{
    m_patterns.emplace_back(64, 8);
    m_patterns.emplace_back(64, 8);
    m_patterns.emplace_back(64, 8);

}

Engine::~Engine()
{
    if (m_initialized)
        shutdown();
}

bool Engine::initialize()
{
    if (m_initialized)
        return true;

    m_transport = std::make_unique<Transport>();

    m_initialized = true;
    return true;
}

void Engine::shutdown()
{
    if (!m_initialized)
        return;

    stop();

    m_transport.reset();

    m_initialized = false;
}

void Engine::start()
{
    if (!m_initialized)
        return;

    m_transport->play();
}

void Engine::stop()
{
    if (!m_initialized)
        return;

    m_transport->stop();
}

void Engine::preview_note(size_t track, uint8_t note)
{
    if (track < m_tracks.size())
        m_tracks[track].note_on(note, 100);
}

Instrument& Engine::instrument(size_t i)
{
    return m_instruments[i];
}

size_t Engine::instrument_count() const
{
    return m_instruments.size();
}

size_t Engine::max_track_latency() const
{
    size_t max = 0;

    for (const auto& t : m_tracks)
        max = std::max(max, t->total_latency());

    return max;
}

Transport& Engine::transport()
{
    return *m_transport;
}

ing ---
        if (m_samples_until_next_row == 0)
        {
            for (size_t t = 0; t < m_tracks.size(); ++t)
            {
                const NoteEvent& ev =
                    m_sequencer.current_event(t);

                if (ev.note != 255)
                    m_tracks[t].note_on(ev.note, 127);

                handle_effect(ev);
            }

            m_sequencer.advance_row();

            if (m_sequencer.current_row() >=
                m_sequencer.row_count())
            {
                if (m_loop_pattern.load())
                {
                    m_sequencer.set_row(0);
                }
                else
                {
                    stop();
                    return;
                }
            }

            m_samples_until_next_row =
                m_timing.samples_per_row();
        }

        size_t block = std::min(
            (size_t)m_samples_until_next_row,
            nframes - processed);

        // --- 4. Clear mix buffers ---
        for (size_t i = 0; i < block; ++i)
        {
            m_mix_l[i] = 0.f;
            m_mix_r[i] = 0.f;
        }

        // --- 5. Render tracks ---
        for (size_t t = 0; t < m_tracks.size(); ++t)
        {
            auto& track = m_tracks[t];

            track.process(
                m_track_l[t],
                m_track_r[t],
                block);

            for (size_t i = 0; i < block; ++i)
            {
                m_mix_l[i] += m_track_l[t][i];
                m_mix_r[i] += m_track_r[t][i];
            }
        }

        // --- 6. Apply master gain ---
        for (size_t i = 0; i < block; ++i)
        {
            float l = m_mix_l[i] * m_master_gain;
            float r = m_mix_r[i] * m_master_gain;

            // optional soft clip
            out_l[processed + i] = soft_clip(l);
            out_r[processed + i] = soft_clip(r);
        }

        processed += block;
        m_samples_until_next_row -= block;
    }
}


void Engine::process_audio(float* out_l,
                           float* out_r,
                           size_t nframes)
{
    EngineCommand cmd;

    while (m_cmd_queue.pop(cmd))
    {
        switch (cmd.type)
        {
            case EngineCommandType::Play:
                m_playing = true;
                break;

            case EngineCommandType::Stop:
                m_playing = false;
                break;

            case EngineCommandType::SetTempo:
                m_timing.set_bpm(cmd.value);
                break;
        }
    }

    if (m_transport.load() !=
        TransportState::Playing &&
        m_transport.load() !=
        TransportState::Recording)
    {
        std::fill(out_l, out_l + nframes, 0.f);
        std::fill(out_r, out_r + nframes, 0.f);
        return;
    }

    size_t processed = 0;

    while (processed < nframes)
    {
        if (m_samples_until_next_tick == 0)
        {
            process_tick();
            m_samples_until_next_tick =
            m_timing.samples_per_tick();
        }

        size_t block = std::min(
            m_samples_until_next_tick,
            nframes - processed);

        render_block(out_l + processed,
                     out_r + processed,
                     block);

        processed += block;
        m_samples_until_next_tick -= block;
    }

}

void Engine::set_active_pattern(size_t index)
{
    if (index < m_patterns.size())
        m_active_pattern.store(index);
}

size_t Engine::active_pattern() const
{
    return m_active_pattern.load();
}

Pattern& Engine::pattern()
{
    return m_patterns[m_active_pattern.load()];
}



inline float Engine::soft_clip(float x)
{
    const float limit = 0.95f;

    if (x > limit)
        return limit;
    if (x < -limit)
        return -limit;

    return x;
}

void Engine::process_audio(float* out_l,
                           float* out_r,
                           size_t nframes)
{
    EngineCommand cmd;

    while (m_cmd_queue.pop(cmd))
    {
        switch (cmd.type)
        {
            case EngineCommandType::Play:
                m_playing = true;
                break;

            case EngineCommandType::Stop:
                m_playing = false;
                break;

            case EngineCommandType::SetTempo:
                m_timing.set_bpm(cmd.value);
                break;
        }
    }

    if (!m_playing)
    {
        memset(out_l, 0, sizeof(float) * nframes);
        memset(out_r, 0, sizeof(float) * nframes);
        return;
    }

    size_t processed = 0;

    while (processed < nframes)
    {
        if (m_samples_until_next_tick == 0)
        {
            process_tick();
            m_samples_until_next_tick =
            m_timing.samples_per_tick();
        }

        size_t block = std::min(
            m_samples_until_next_tick,
            nframes - processed);

        render_block(out_l + processed,
                     out_r + processed,
                     block);

        processed += block;
        m_samples_until_next_tick -= block;
    }
}

void Engine::render_block(float* out_l,
                          float* out_r,
                          size_t frames)
{
    for (size_t i = 0; i < frames; ++i)
    {
        m_mix_l[i] = 0.f;
        m_mix_r[i] = 0.f;
    }

    for (size_t t = 0; t < m_tracks.size(); ++t)
    {
        auto& track = m_tracks[t];

        track.process(
            m_track_l[t],
            m_track_r[t],
            frames);

        for (size_t i = 0; i < frames; ++i)
        {
            m_mix_l[i] += m_track_l[t][i];
            m_mix_r[i] += m_track_r[t][i];
        }
    }

    for (size_t i = 0; i < frames; ++i)
    {
        out_l[i] = soft_clip(m_mix_l[i] * m_master_gain);
        out_r[i] = soft_clip(m_mix_r[i] * m_master_gain);
    }
}

void Engine::handle_effect_row_start(size_t t,
                                     const NoteEvent& ev)
{
    auto& track = m_tracks[t];

    uint8_t fx = ev.effect;
    uint8_t param = ev.param;

    switch (fx)
    {
        // Bxx Pattern Jump
        case 0x0B:
        {
            m_sequencer.jump_to_order(param);
            break;
        }

        // Dxx Pattern Break
        case 0x0D:
        {
            int row =
            ((param >> 4) * 10) + (param & 0x0F);

            m_sequencer.break_to_row(row);
            break;
        }

        // Fxx
        case 0x0F:
        {
            if (param < 32)
                m_timing.set_speed(param);
            else
                m_timing.set_bpm(param);
            break;
        }

        // Cxx set volume
        case 0x0C:
        {
            track.set_volume(param / 64.f);
            break;
        }

        // Axy volume slide
        case 0x0A:
        {
            track.m_fx_state.vol_slide_up =
            (param >> 4);

            track.m_fx_state.vol_slide_down =
            (param & 0x0F);

            break;
        }

        // 3xx portamento
        case 0x03:
        {
            track.m_fx_state.porta_speed =
            param * 0.0005f;  // scaled

            track.m_fx_state.porta_active = true;
            break;
        }

        // E9x retrig
        case 0x0E:
        {
            uint8_t sub = param >> 4;
            uint8_t val = param & 0x0F;

            if (sub == 0x9)
            {
                track.m_fx_state.retrig_ticks = val;
                track.m_fx_state.retrig_counter = 0;
            }
            else if (sub == 0xC)
            {
                track.m_fx_state.note_cut_tick = val;
            }
            break;
        }
    }
}


void Engine::handle_effect(const NoteEvent& ev)
{
    switch (ev.effect)
    {
        case EffectType::SetTempo:
            m_timing.set_tempo(ev.effect_param);
            break;

        case EffectType::Volume:
            // apply to current track
            break;

        case EffectType::PatternBreak:
            m_sequencer.pattern_break();
            break;

        case EffectType::Jump:
            m_sequencer.jump_to(ev.effect_param);
            break;

        case EffectType::NoteCut:
            // schedule note off inside row
            break;

        default:
            break;
    }
}

void Engine::handle_midi(uint8_t* data,
                         size_t size)
{
    if (size < 3)
        return;

    uint8_t status = data[0] & 0xF0;

    if (status == 0x90 && data[2] > 0)
    {
        m_tracks[0].note_on(data[1], data[2]);
    }
    else if (status == 0x80 ||
        (status == 0x90 && data[2] == 0))
    {
        m_tracks[0].note_off();
    }
}

UndoStack& Engine::undo_stack()
{
    return m_undo;
}
BlockClipboard& Engine::clipboard()
{
    return m_clipboard;
}

void Engine::play()
{
    m_transport.store(
        TransportState::Playing);
}

void Engine::stop()
{
    m_transport.store(
        TransportState::Stopped);
}

void Engine::toggle_play()
{
    if (m_transport.load() ==
        TransportState::Playing)
        stop();
    else
        play();
}

TransportState Engine::transport() const
{
    return m_transport.load();
}

void Engine::set_loop(bool enable)
{
    m_loop_pattern.store(enable);
}

void Engine::set_play_position(
    size_t pattern,
    size_t row)
{
    if (pattern < m_patterns.size())
        m_active_pattern.store(pattern);

    m_sequencer.set_row(row);

    m_samples_until_next_row =
    m_timing.samples_per_row();
}


} // namespace dg
