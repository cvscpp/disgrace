#pragma once

#include <atomic>
#include <array>

namespace dg
{

template<typename T, size_t Size>
class RingBuffer
{
public:
    bool push(const T &item)
    {
        auto head = m_head.load(std::memory_order_relaxed);
        auto next = (head + 1) % Size;

        if (next == m_tail.load(std::memory_order_acquire))
            return false; // full

        m_buffer[head] = item;
        m_head.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T &item)
    {
        auto tail = m_tail.load(std::memory_order_relaxed);

        if (tail == m_head.load(std::memory_order_acquire))
            return false; // empty

        item = m_buffer[tail];
        m_tail.store((tail + 1) % Size, std::memory_order_release);
        return true;
    }

private:
    std::array<T, Size> m_buffer;
    std::atomic<size_t> m_head{0};
    std::atomic<size_t> m_tail{0};
};

}
