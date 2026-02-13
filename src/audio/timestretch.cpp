#include "timestretch.h"
#include <soundtouch/SoundTouch.h>

namespace dg
{

    bool TimeStretch::stretch(const std::vector<float>& input,
                              std::vector<float>& output,
                              float tempo_ratio)
    {
        if (input.empty() || tempo_ratio <= 0.0f)
            return false;

        soundtouch::SoundTouch st;
        st.setChannels(1);
        st.setSampleRate(44100); // TODO: parameterize
        st.setTempo(tempo_ratio);

        st.putSamples(input.data(), input.size());

        output.clear();
        std::vector<float> temp(4096);

        while (true)
        {
            uint32_t received =
            st.receiveSamples(temp.data(), temp.size());

            if (received == 0)
                break;

            output.insert(output.end(),
                          temp.begin(),
                          temp.begin() + received);
        }

        return true;
    }

}
