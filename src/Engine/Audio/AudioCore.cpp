#include "AudioCore.hpp"

bool AudioCore::Init()
{
    return m_SoundPlayer.Init();
}

void AudioCore::PlayLoopingSound(Entity entity, const std::string& soundPath, float volume)
{
    ECSCore* pECS = ECSCore::GetInstance();

    SoundComponent soundComponent = m_SoundPlayer.CreateSound(soundPath);
    if (soundComponent.pSound) {
        pECS->AddComponent(entity, soundComponent);
        pECS->AddComponent(entity, m_SoundPlayer.CreateSoundLooper(soundComponent));

        m_SoundPlayer.PlaySound(soundComponent);
        m_SoundPlayer.SetVolume(soundComponent, volume);
    }
}
