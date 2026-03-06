#pragma once
#include <cstdint>
#include <cstddef> // Add this line

namespace disgrace_ns
{

    class Voice
    {
    public:
        virtual ~Voice() = default;

        virtual void start(uint8_t note,
                           uint8_t velocity,
                           float frequency,
                           size_t offset_samples = 0) = 0;

                           virtual void stop() = 0;
                           virtual void panic() = 0;

                           virtual void set_pitch(float freq) = 0;
                           virtual void set_volume(float vol) = 0;

                           virtual void process(float* out_l,
                                                float* out_r,
                                                size_t frames) = 0;

                                                virtual bool active() const = 0;

        void set_column(size_t col) { m_column = col; }
        size_t column() const { return m_column; }

    protected:
        size_t m_column = 0;
    };

} // namespace disgrace_ns
