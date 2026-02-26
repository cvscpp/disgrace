#include "sample_instrument.h"

namespace disgrace_ns
{

    disgrace_ns::SampleInstrument::SampleInstrument(
        disgrace_ns::SampleData* data,
        double engine_rate)
    : m_sample(data),
    m_engine_rate(engine_rate)
    {
    }

    ::std::unique_ptr<disgrace_ns::Voice>
    disgrace_ns::SampleInstrument::create_voice()
    {
        return ::std::make_unique<disgrace_ns::SampleVoice>(
            m_sample,
            m_engine_rate);
    }

} // namespace disgrace_ns
