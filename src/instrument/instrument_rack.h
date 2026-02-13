#pragma once
#include <vector>
#include <memory>

namespace dg
{

    class Instrument;

    class InstrumentRack
    {
    public:
        size_t add(std::unique_ptr<Instrument> inst);

        Instrument* get(size_t index);

        size_t size() const;

    private:
        std::vector<std::unique_ptr<Instrument>> m_instruments;
    };

}
