#pragma once

#include <FMOD/core/fmod.hpp>

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/IDVector.hpp>

struct Sound {
    FMOD::Sound* pSound;
    FMOD::Channel* pChannel;
};

const std::type_index tid_sound = TID(Sound);

class SoundHandler : public ComponentHandler
{
public:
    SoundHandler(ECSCore* pECS);
    ~SoundHandler();

    virtual bool init() override;

    bool createSound(Entity entity, const std::string& fileName);
    bool playSound(Entity entity);

    FMOD::System* getSystem() { return m_pSystem; }

public:
    IDDVector<Sound> m_Sounds;

private:
    FMOD::System* m_pSystem;
};
