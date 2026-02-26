#pragma once

#include <string>

namespace disgrace_ns
{

    class Engine;

    class SongSerializer
    {
    public:
        static bool save(const disgrace_ns::Engine& engine,
                         const ::std::string& path);

        static bool load(disgrace_ns::Engine& engine,
                         const ::std::string& path);
    };

} // namespace disgrace_ns
