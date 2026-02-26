#pragma once
#include <atomic>
#include <array>
#include <cstddef> // For size_t

namespace disgrace_ns
{

template<typename T, size_t Size>
class MidiQueue
{
public:
    bool push(const T& v)
    {
        auto next =
        (m_write + 1) % Size;

        if (next == m_read)
            return false;

        m_buffer[m_write] = v;
        m_write = next;
        return true;
    }

    bool pop(T& out)
    {
        if (m_read == m_write)
            return false;

        out = m_buffer[m_read];
        m_read =
        (m_read + 1) % Size;

        return true;
    }

private:
    ::std::array<T, Size> m_buffer{};
    ::std::atomic<size_t> m_write{0};
    ::std::atomic<size_t> m_read{0};
};

} // namespace disgrace_ns
