#pragma once
#include "instrument.h"
#include "../audio/sample_voice.h"

namespace disgrace_ns
{

    class SampleInstrument : public disgrace_ns::Instrument
    {
    public:
        SampleInstrument(disgrace_ns::SampleData* data,
                         double engine_rate);

        void note_on(uint8_t note, uint8_t velocity) override;
        void note_off() override;
        void set_volume(float vol) override;
        void set_pitch(float freq) override;
        void process(float* l, float* r, size_t nframes) override;

    protected:
        ::std::unique_ptr<disgrace_ns::Voice> create_voice() override;

    private:
        disgrace_ns::SampleData* m_sample;
        double m_engine_rate;
    };

} // namespace disgrace_ns
