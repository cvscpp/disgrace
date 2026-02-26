#include "midi_input.h"
#include <functional> // For std::function
#include <thread>     // For std::thread, std::this_thread
#include <atomic>     // For std::atomic
#include <chrono>     // For std::chrono
#include <utility>    // For std::move

namespace disgrace_ns
{

disgrace_ns::MidiInput::MidiInput() {}
disgrace_ns::MidiInput::~MidiInput() { stop(); }

void disgrace_ns::MidiInput::start()
{
    m_running = true;
    m_thread = ::std::thread(&MidiInput::run, this);
}

void disgrace_ns::MidiInput::stop()
{
    m_running = false;
    if (m_thread.joinable())
        m_thread.join();
}

void disgrace_ns::MidiInput::set_callback(Callback cb)
{
    m_callback = ::std::move(cb);
}

void disgrace_ns::MidiInput::run()
{
    while (m_running)
    {
        ::std::this_thread::sleep_for(
            ::std::chrono::milliseconds(1));

        // platform MIDI read here later
    }
}

} // namespace disgrace_ns
