#pragma once

#include <Engine/Audio/SoundHandler.hpp>
#include <Engine/Audio/SoundPlayer.hpp>

class AudioCore
{
public:
    AudioCore(ECSCore* pECS);
    ~AudioCore();

private:
    // Component Handlers
    SoundHandler m_SoundHandler;

    // Systems
    SoundPlayer m_SoundPlayer;
};
