#include "instrument_rack.h"

namespace dg
{

    size_t InstrumentRack::add(
        std::unique_ptr<Instrument> inst)
    {
        m_instruments.push_back(std::move(inst));
        return m_instruments.size() - 1;
    }

    Instrument* InstrumentRack::get(size_t index)
    {
        if (index >= m_instruments.size())
            return nullptr;

        return m_instruments[index].get();
    }

    size_t InstrumentRack::size() const
    {
        return m_instruments.size();
    }

}
