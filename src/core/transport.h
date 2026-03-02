#pragma once

#include <atomic>
#include <cstddef> 

namespace disgrace_ns
{
    class Engine;

    enum class TransportState
    {
        Stopped,
        Playing,
        Recording
    };

class Transport
{
public:
    Transport(Engine& engine);

    bool is_playing() const;

    void set_tempo(double bpm);
    double tempo() const;
    void play();
    void stop();
    void record(); 
    void toggle_play();
    void set_loop(bool enable);

    TransportState state() const; 

    void set_play_position(size_t pattern,
                           size_t row);

private:
    Engine& m_engine;
    ::std::atomic<double> m_tempo;
    ::std::atomic<TransportState> m_transport{TransportState::Stopped};
    ::std::atomic<bool> m_loop_pattern{true};
};

} // namespace disgrace_ns
