#pragma once

#include <atomic>

namespace dg
{
    enum class TransportState
    {
        Stopped,
        Playing,
        Recording
    };

class Transport
{
public:
    Transport();

    void play();
    void stop();

    bool is_playing() const;

    void set_tempo(double bpm);
    double tempo() const;
    void play();
    void stop();
    void toggle_play();
    void set_loop(bool enable);

    TransportState transport() const;

    void set_play_position(size_t pattern,
                           size_t row);

private:
    std::atomic<double> m_tempo;
    std::atomic<TransportState> m_transport{TransportState::Stopped};
    std::atomic<bool> m_loop_pattern{true};


};

} // namespace dg
