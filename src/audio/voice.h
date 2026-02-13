#pragma once
#include <cstdint>

namespace dg
{

    class Voice
    {
    public:
        virtual ~Voice() = default;

        virtual void start(uint8_t note,
                           uint8_t velocity,
                           float frequency) = 0;

                           virtual void stop() = 0;

                           virtual void set_pitch(float freq) = 0;
                           virtual void set_volume(float vol) = 0;

                           virtual void process(float* out_l,
                                                float* out_r,
                                                size_t frames) = 0;

                                                virtual bool active() const = 0;
    };

}
