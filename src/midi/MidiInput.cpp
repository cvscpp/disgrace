#include "MidiInput.h"
#include <chrono>

MidiInput::MidiInput() {}
MidiInput::~MidiInput() { stop(); }

void MidiInput::start()
{
    m_running = true;
    m_thread = std::thread(&MidiInput::run, this);
}

void MidiInput::stop()
{
    m_running = false;
    if (m_thread.joinable())
        m_thread.join();
}

void MidiInput::set_callback(Callback cb)
{
    m_callback = std::move(cb);
}

void MidiInput::run()
{
    while (m_running)
    {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(1));

        // platform MIDI read here later
    }
}
