namespace disgrace_ns
{

enum class EngineCommandType
{
    Play,
    Stop,
    SetTempo,
    PlayPattern,
    ResizePattern
};

struct EngineCommand
{
    EngineCommandType type;
    size_t index;
    size_t value;
};

} // namespace disgrace_ns
