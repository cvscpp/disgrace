#pragma once
#include <cstddef>
#include <cstdint> // Added for uint8_t
#include <memory>  // Added for std::unique_ptr
#include <array>   // Added for std::array
#include "../audio/voice.h" // Added for disgrace_ns::Voice

namespace disgrace_ns
{

constexpr size_t MAX_VOICES = 32;

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
    virtual ::std::unique_ptr<disgrace_ns::Voice> create_voice() = 0;

    disgrace_ns::Voice* allocate_voice();

protected:
    ::std::array<::std::unique_ptr<disgrace_ns::Voice>, MAX_VOICES> m_voices;
};

} // namespace disgrace_ns
