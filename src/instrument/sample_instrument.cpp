#include "sample_instrument.h"

namespace dg
{

    SampleInstrument::SampleInstrument(
        SampleData* data,
        double engine_rate)
    : m_sample(data),
    m_engine_rate(engine_rate)
    {
    }

    std::unique_ptr<Voice>
    SampleInstrument::create_voice()
    {
        return std::make_unique<SampleVoice>(
            m_sample,
            m_engine_rate);
    }

}
