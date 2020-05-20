#pragma once

#define NOMINMAX

#include <Engine/ECS/System.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <DirectXMath.h>

class LightHandler;
class SoundHandler;
class TransformHandler;
class VelocityHandler;
struct Sound;

class SoundPlayer : public System
{
public:
    SoundPlayer(ECSCore* pECS);
    ~SoundPlayer();

    virtual bool initSystem() override;

    virtual void update(float dt) override;

private:
    void stereoPan(Sound& sound, const DirectX::XMVECTOR& camToSound, const DirectX::XMVECTOR& camDirFlat);
    void dopplerEffect(Sound& sound, const DirectX::XMVECTOR& camPos, const DirectX::XMVECTOR& camVelocity, float camSpeed, const DirectX::XMVECTOR& objectPos, const DirectX::XMFLOAT3& objectVelocity);

private:
    IDVector m_Sounds;
    IDVector m_LoopedSounds;
    IDVector m_Cameras;

    LightHandler* m_pLightHandler;
    SoundHandler* m_pSoundHandler;
    TransformHandler* m_pTransformHandler;
    VelocityHandler* m_pVelocityHandler;
};
