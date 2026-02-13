#include "transport.h"

namespace dg
{

Transport::Transport()
    : m_playing(false),
      m_tempo(120.0)
{
}

void Transport::play()
{
    m_playing.store(true, std::memory_order_relaxed);
}

void Transport::stop()
{
    m_playing.store(false, std::memory_order_relaxed);
}

bool Transport::is_playing() const
{
    return m_playing.load(std::memory_order_relaxed);
}

void Transport::set_tempo(double bpm)
{
    m_tempo.store(bpm, std::memory_order_relaxed);
}

double Transport::tempo() const
{
    return m_tempo.load(std::memory_order_relaxed);
}

} // namespace dg
