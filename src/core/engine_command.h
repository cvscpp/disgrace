enum class EngineCommandType
{
    Play,
    Stop,
    SetTempo
};

struct EngineCommand
{
    EngineCommandType type;
    float value;
};
