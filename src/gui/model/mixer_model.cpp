#include "mixer_model.h"

namespace dg
{

    void MixerModel::update_from_engine(
        const std::vector<MixerChannelState>& state)
    {
        m_channels = state;
    }

    const std::vector<MixerChannelState>&
    MixerModel::channels() const
    {
        return m_channels;
    }

}
