#pragma once
#include "instrument.h"
#include "../audio/sample_voice.h"

namespace disgrace_ns
{

    struct SampleEntry {
        std::string name;
        std::shared_ptr<disgrace_ns::SampleData> data;
    };

    class SampleInstrument : public disgrace_ns::Instrument
    {
    public:
        SampleInstrument(double engine_rate);

        void note_on(uint8_t note, uint8_t velocity) override;
        void note_off() override;
        void set_volume(float vol) override;
        void set_pitch(float freq) override;
        void process(float* l, float* r, size_t nframes) override;

        void add_sample(const std::string& name, std::shared_ptr<disgrace_ns::SampleData> data);
        void remove_sample(size_t index);
        void move_sample(size_t from, size_t to);
        void set_sample_name(size_t index, const std::string& name);
        size_t sample_count() const { return m_samples.size(); }
        const SampleEntry& get_sample(size_t index) const { return m_samples[index]; }
        SampleEntry& get_sample(size_t index) { return m_samples[index]; }

    protected:
        ::std::unique_ptr<disgrace_ns::Voice> create_voice() override;

    private:
        std::vector<SampleEntry> m_samples;
        double m_engine_rate;
    };

} // namespace disgrace_ns
