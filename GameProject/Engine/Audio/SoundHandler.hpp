#pragma once

#include <FMOD/core/fmod.hpp>

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/IDVector.hpp>

struct Sound {
    FMOD::Sound* pSound;
    FMOD::Channel* pChannel;
};

struct SoundLooper {
    float NextLoopCountdown;
};

const std::type_index g_TIDSound        = TID(Sound);
const std::type_index g_TIDSoundLooper  = TID(SoundLooper);

class SoundHandler : public ComponentHandler
{
public:
    SoundHandler(ECSCore* pECS);
    ~SoundHandler();

    virtual bool initHandler() override;

    bool createSound(Entity entity, const std::string& fileName);
    bool playSound(Entity entity);
    bool setVolume(Entity entity, float volume);
    bool loopSound(Entity entity);

    // Duration is in seconds
    float getSoundDuration(Entity entity);

    inline Sound& getSound(Entity entity)               { return m_Sounds.indexID(entity); }
    inline bool hasSoundLooper(Entity entity)           { return m_SoundLoopers.hasElement(entity); }
    inline SoundLooper& getSoundLooper(Entity entity)   { return m_SoundLoopers.indexID(entity); }
    FMOD::System* getSystem() { return m_pSystem; }

private:
    IDDVector<Sound> m_Sounds;
    IDDVector<SoundLooper> m_SoundLoopers;

    FMOD::System* m_pSystem;
};
