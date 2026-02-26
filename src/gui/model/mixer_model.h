#pragma once
#include <vector>

namespace disgrace_ns
{

    struct MixerChannelState
    {
        float volume;
        float pan;
        float peak_l;
        float peak_r;
    };

    class MixerModel
    {
    public:
        void update_from_engine(const ::std::vector<disgrace_ns::MixerChannelState>& state);

        const ::std::vector<disgrace_ns::MixerChannelState>& channels() const;

    private:
        ::std::vector<disgrace_ns::MixerChannelState> m_channels;
    };

} // namespace disgrace_ns
