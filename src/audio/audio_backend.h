#pragma once

#include <cstdint>

namespace disgrace_ns
{

class AudioBackend
{
public:
    virtual ~AudioBackend() = default;

    virtual bool start() = 0;
    virtual void stop() = 0;

    virtual uint32_t sample_rate() const = 0;
    virtual uint32_t buffer_size() const = 0;
};

} // namespace disgrace_ns
