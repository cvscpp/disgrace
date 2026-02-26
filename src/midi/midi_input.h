#pragma once
#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <cstdint>

namespace disgrace_ns
{

struct MidiMessage
{
    uint8_t status;
    uint8_t data1;
    uint8_t data2;
};

class MidiInput
{
public:
    using Callback =
    ::std::function<void(const MidiMessage&)>;

    MidiInput();
    ~MidiInput();

    void start();
    void stop();

    void set_callback(Callback cb);

private:
    void run();

    ::std::thread m_thread;
    ::std::atomic<bool> m_running{false};
    Callback m_callback;
};

} // namespace disgrace_ns
