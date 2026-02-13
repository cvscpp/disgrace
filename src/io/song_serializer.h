#pragma once

#include <string>

namespace dg
{

    class Engine;

    class SongSerializer
    {
    public:
        static bool save(const Engine& engine,
                         const std::string& path);

        static bool load(Engine& engine,
                         const std::string& path);
    };

}
