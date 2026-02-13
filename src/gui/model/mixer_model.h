#pragma once
#include <vector>

namespace dg
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
        void update_from_engine(const std::vector<MixerChannelState>& state);

        const std::vector<MixerChannelState>& channels() const;

    private:
        std::vector<MixerChannelState> m_channels;
    };

}
