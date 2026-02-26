#pragma once
#include <string>

namespace disgrace_ns
{

    class ProjectArchive
    {
    public:
        static bool save(const ::std::string& path,
                         const ::std::string& temp_folder);

        static bool extract(const ::std::string& path,
                            const ::std::string& dest_folder);
    };

} // namespace disgrace_ns
