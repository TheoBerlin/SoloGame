#pragma once

#include <Engine/ECS/System.hpp>
#include <Engine/Utils/IDVector.hpp>

class LightHandler;
class SoundHandler;
class TransformHandler;

class SoundPlayer : public System
{
public:
    SoundPlayer(ECSCore* pECS);
    ~SoundPlayer();

    virtual bool initSystem() override;

    virtual void update(float dt) override;

private:
    IDVector m_Sounds;
    IDVector m_Cameras;

    LightHandler* m_pLightHandler;
    SoundHandler* m_pSoundHandler;
    TransformHandler* m_pTransformHandler;
};
