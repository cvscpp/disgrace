#pragma once

#include <string>
#include <vector>
#include <cstdint> // Added for uint32_t

namespace disgrace_ns
{

    class AudioFile
    {
    public:
        static bool load_wav(const ::std::string& path,
                             ::std::vector<float>& left,
                             ::std::vector<float>& right,
                             uint32_t& sample_rate);

        static bool save_wav(const ::std::string& path,
                             const ::std::vector<float>& left,
                             const ::std::vector<float>& right,
                             uint32_t sample_rate);
    };

} // namespace disgrace_ns
