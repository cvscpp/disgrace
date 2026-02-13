#pragma once
#include <cstddef>

namespace dg
{

class Instrument
{
public:
    virtual ~Instrument() = default;

    virtual void note_on(uint8_t note, uint8_t velocity) = 0;
    virtual void note_off() = 0;

    virtual void set_volume(float vol) = 0;

    virtual void set_pitch(float freq) = 0;
    virtual void process(float* l,
                         float* r,
                         size_t nframes) = 0;

protected:
    virtual std::unique_ptr<Voice> create_voice() = 0;

    Voice* allocate_voice();

protected:
    std::array<std::unique_ptr<Voice>, MAX_VOICES> m_voices;
};

}
