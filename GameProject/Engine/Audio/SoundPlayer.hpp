#pragma once

#include <Engine/ECS/System.hpp>
#include <Engine/Utils/IDVector.hpp>

class SoundHandler;

class SoundPlayer : public System
{
public:
    SoundPlayer(ECSCore* pECS);
    ~SoundPlayer();

    virtual bool init() override;

    virtual void update(float dt) override;

private:
    IDVector<Entity> m_Sounds;

    SoundHandler* m_pSoundHandler;
};
