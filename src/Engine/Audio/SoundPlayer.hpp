#pragma once

#include <Engine/ECS/System.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <fmod.hpp>

#include <DirectXMath.h>

struct SoundComponent {
    DECL_COMPONENT(SoundComponent);
    FMOD::Sound* pSound;
    FMOD::Channel* pChannel;
};

struct SoundLooperComponent {
    DECL_COMPONENT(SoundLooperComponent);
    float NextLoopCountdown;
};

class SoundPlayer : public System
{
public:
    SoundPlayer();
    ~SoundPlayer();

    bool Init();

    void Update(float dt) override final;

    SoundComponent CreateSound(const std::string& fileName);
    bool PlaySound(SoundComponent& sound);
    bool SetVolume(SoundComponent& sound, float volume);
    SoundLooperComponent CreateSoundLooper(const SoundComponent& sound);

    // Duration is in seconds
    float GetSoundDuration(const SoundComponent& sound);

    FMOD::System* GetSystem() { return m_pSystem; }

private:
    void stereoPan(SoundComponent& sound, const DirectX::XMVECTOR& camToSound, const DirectX::XMVECTOR& camDirFlat);
    void dopplerEffect(SoundComponent& sound, const DirectX::XMVECTOR& camPos, const DirectX::XMVECTOR& camVelocity, float camSpeed, const DirectX::XMVECTOR& objectPos, const DirectX::XMFLOAT3& objectVelocity);

private:
    FMOD::System* m_pSystem;

    IDVector m_Sounds;
    IDVector m_LoopedSounds;
    IDVector m_Cameras;
};
