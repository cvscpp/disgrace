#pragma once
#include <vector>
#include <memory>

namespace disgrace_ns
{

    class Instrument;

    class InstrumentRack
    {
    public:
        size_t add(::std::unique_ptr<disgrace_ns::Instrument> inst);

        Instrument* get(size_t index);

        size_t size() const;

    private:
        ::std::vector<::std::unique_ptr<disgrace_ns::Instrument>> m_instruments;
    };

} // namespace disgrace_ns
