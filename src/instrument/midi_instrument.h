#pragma once
#include "instrument.h"

namespace disgrace_ns {

class Engine;

class MidiInstrument : public Instrument {
public:
    MidiInstrument(Engine* engine) : m_engine(engine), m_channel(0), m_program(0) {}

    void note_on(uint8_t note, uint8_t velocity) override;
    void note_off() override;
    void set_volume(float vol) override;
    void set_pitch(float freq) override;
    void process(float* l, float* r, size_t nframes) override;

    void set_channel(int ch);
    int channel() const { return m_channel; }

    void set_program(int prog);
    int program() const { return m_program; }

protected:
    std::unique_ptr<Voice> create_voice() override { return nullptr; }

private:
    Engine* m_engine;
    int m_channel;
    int m_program;
};

} // namespace disgrace_ns
