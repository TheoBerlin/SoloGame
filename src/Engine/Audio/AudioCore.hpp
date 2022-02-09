#pragma once

#include <Engine/Audio/SoundPlayer.hpp>

class AudioCore
{
public:
    AudioCore() = default;
    ~AudioCore() = default;

    bool Init();

    void PlayLoopingSound(Entity entity, const std::string& soundPath, float volume);

    SoundPlayer* GetSoundPlayer() { return &m_SoundPlayer; }

private:
    // Systems
    SoundPlayer m_SoundPlayer;
};
