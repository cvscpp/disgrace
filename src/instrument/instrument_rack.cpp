#include "instrument_rack.h"
#include "instrument.h" // Added for disgrace_ns::Instrument definition

namespace disgrace_ns
{

    size_t disgrace_ns::InstrumentRack::add(
        ::std::unique_ptr<disgrace_ns::Instrument> inst)
    {
        m_instruments.push_back(::std::move(inst));
        return m_instruments.size() - 1;
    }

    disgrace_ns::Instrument* disgrace_ns::InstrumentRack::get(size_t index)
    {
        if (index >= m_instruments.size())
            return nullptr;

        return m_instruments[index].get();
    }

    size_t disgrace_ns::InstrumentRack::size() const
    {
        return m_instruments.size();
    }

} // namespace disgrace_ns
