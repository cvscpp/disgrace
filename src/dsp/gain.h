#pragma once
#include "dsp.h"

namespace dg
{

class GainDSP : public DSP
{
public:
    float gain = 1.0f;

    void process(float* l,
                 float* r,
                 size_t nframes) override
    {
        for (size_t i = 0; i < nframes; ++i)
        {
            l[i] *= gain;
            r[i] *= gain;
        }
    }
};

}
