#pragma once
#include "voice.h"
#include "sample_data.h"
#include "adsr.h"

namespace disgrace_ns
{

    class SampleVoice : public Voice
    {
    public:
        SampleVoice(disgrace_ns::SampleData* data,
                    double engine_rate);

        void start(uint8_t note,
                   uint8_t velocity,
                   float freq) override;

                   void stop() override;

                   void set_pitch(float freq) override;
                   void set_volume(float vol) override;

                   void process(float* out_l,
                                float* out_r,
                                size_t frames) override;

                                bool active() const override;

    private:
        disgrace_ns::SampleData* m_sample;
        disgrace_ns::ADSR m_env;

        double m_engine_rate;
        double m_position = 0.0;
        double m_increment = 1.0;

        float m_volume = 1.f;
        bool m_active = false;
    };

} // namespace disgrace_ns
