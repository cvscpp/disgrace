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

    protected:
        ::std::unique_ptr<disgrace_ns::Voice> create_voice() override;

    private:
        disgrace_ns::SampleData* m_sample;
        double m_engine_rate;
    };

} // namespace disgrace_ns
