#include <nlohmann/json.hpp>
#include <fstream>
#include "engine.h" // Include engine.h to get the class definition

namespace disgrace_ns
{

void Engine::save_project(
    const ::std::string& path)
{
    nlohmann::json j;

    j["tempo"] = m_timing.tempo();

    for (auto& pat : m_patterns)
        j["patterns"].push_back(
            pat.to_json());

    ::std::ofstream f(path);
    f << j.dump(2);
}

} // namespace disgrace_ns
