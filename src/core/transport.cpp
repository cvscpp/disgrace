#include "transport.h"

namespace disgrace_ns
{

Transport::Transport()
    : m_tempo(120.0), // Default tempo
      m_transport(TransportState::Stopped),
      m_loop_pattern(true)
{
}

bool Transport::is_playing() const
{
    // This can be derived from the state()
    return m_transport.load(::std::memory_order_relaxed) == TransportState::Playing ||
           m_transport.load(::std::memory_order_relaxed) == TransportState::Recording;
}

void Transport::set_tempo(double bpm)
{
    m_tempo.store(bpm, ::std::memory_order_relaxed);
}

double Transport::tempo() const
{
    return m_tempo.load(::std::memory_order_relaxed);
}

void Transport::play()
{
    m_transport.store(TransportState::Playing, ::std::memory_order_relaxed);
}

void Transport::stop()
{
    m_transport.store(TransportState::Stopped, ::std::memory_order_relaxed);
}

void Transport::record()
{
    m_transport.store(TransportState::Recording, ::std::memory_order_relaxed);
}

void Transport::toggle_play()
{
    if (m_transport.load(::std::memory_order_relaxed) == TransportState::Playing)
    {
        stop();
    }
    else
    {
        play();
    }
}

void Transport::set_loop(bool enable)
{
    m_loop_pattern.store(enable, ::std::memory_order_relaxed);
}

TransportState Transport::state() const
{
    return m_transport.load(::std::memory_order_relaxed);
}

void Transport::set_play_position(size_t pattern,
                           size_t row)
{
    // Implementation for setting play position
    // This will likely involve communicating with the sequencer or engine directly
    // For now, leave empty or add a placeholder for future implementation
}

} // namespace disgrace_ns
