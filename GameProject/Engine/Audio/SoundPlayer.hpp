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

    virtual bool init() override;

    virtual void update(float dt) override;

private:
    IDVector<Entity> m_Sounds;
    IDVector<Entity> m_Cameras;

    LightHandler* m_pLightHandler;
    SoundHandler* m_pSoundHandler;
    TransformHandler* m_pTransformHandler;
};