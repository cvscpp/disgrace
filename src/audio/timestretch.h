#include <vector> // Add this line

namespace disgrace_ns // Add this line
{

class TimeStretch
{
public:
    static bool stretch(const ::std::vector<float>& input,
                        ::std::vector<float>& output,
                        float tempo_ratio);
};

} // namespace disgrace_ns // Add this line
