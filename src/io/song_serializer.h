#pragma once

#include <string>

namespace disgrace_ns
{

    class Engine;

    class SongSerializer
    {
    public:
        static bool save(const disgrace_ns::Engine& engine,
                         const ::std::string& folder);

        static bool load(disgrace_ns::Engine& engine,
                         const ::std::string& folder);
    };

} // namespace disgrace_ns
