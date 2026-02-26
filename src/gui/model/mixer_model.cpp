#include "mixer_model.h"

namespace disgrace_ns
{

    void disgrace_ns::MixerModel::update_from_engine(
        const ::std::vector<disgrace_ns::MixerChannelState>& state)
    {
        m_channels = state;
    }

    const ::std::vector<disgrace_ns::MixerChannelState>&
    disgrace_ns::MixerModel::channels() const
    {
        return m_channels;
    }

} // namespace disgrace_ns
