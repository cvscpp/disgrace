#pragma once
#include "instrument.h"
#include "sample_voice.h"

namespace dg
{

    class SampleInstrument : public Instrument
    {
    public:
        SampleInstrument(SampleData* data,
                         double engine_rate);

    protected:
        std::unique_ptr<Voice> create_voice() override;

    private:
        SampleData* m_sample;
        double m_engine_rate;
    };

}
