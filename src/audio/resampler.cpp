#include "resampler.h"
#include <samplerate.h>
#include <vector>

namespace disgrace_ns
{

    bool disgrace_ns::Resampler::process(const ::std::vector<float>& input,
                            ::std::vector<float>& output,
                            double ratio)
    {
        if (input.empty() || ratio <= 0.0)
            return false;

        SRC_DATA data{};
        data.data_in = input.data();
        data.input_frames = input.size();
        data.src_ratio = ratio;

        size_t out_frames = static_cast<size_t>(input.size() * ratio) + 1;
        output.resize(out_frames);

        data.data_out = output.data();
        data.output_frames = out_frames;

        int err = src_simple(&data,
                             SRC_SINC_FASTEST,
                             1);

        if (err != 0)
            return false;

        output.resize(data.output_frames_gen);
        return true;
    }

} // namespace disgrace_ns
