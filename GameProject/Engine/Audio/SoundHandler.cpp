#include "SoundHandler.hpp"

#include <FMOD/core/fmod_errors.h>

#include <Engine/Utils/Logger.hpp>

#include <string>

SoundHandler::SoundHandler(ECSCore* pECS)
    :ComponentHandler(pECS, TID(SoundHandler)),
    m_pSystem(nullptr)
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    handlerReg.ComponentRegistrations = {
        {tid_sound, &m_Sounds, [this](Entity entity){ m_Sounds.indexID(entity).pSound->release(); }}
    };
    registerHandler(handlerReg);
}

SoundHandler::~SoundHandler()
{
    m_pSystem->release();
}

bool SoundHandler::init()
{
    FMOD_RESULT result = FMOD::System_Create(&m_pSystem);
    if (result != FMOD_OK) {
        LOG_ERROR("Failed to create FMOD system: %s", FMOD_ErrorString(result));
        return false;
    }

    const int maxChannels = 512;

    result = m_pSystem->init(maxChannels, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK) {
        LOG_ERROR("Failed to initialize FMOD system: %s", FMOD_ErrorString(result));
        return false;
    }

    return true;
}

bool SoundHandler::createSound(Entity entity, const std::string& fileName)
{
    Sound newSound = {};
    FMOD_RESULT result = m_pSystem->createSound(fileName.c_str(), FMOD_DEFAULT, nullptr, &newSound.pSound);
    if (result != FMOD_OK) {
        LOG_WARNING("Failed to create sound component from file [%s]: %s", fileName.c_str(), FMOD_ErrorString(result));
        return false;
    }

    m_Sounds.push_back(newSound, entity);
    registerComponent(entity, tid_sound);
    return true;
}

bool SoundHandler::playSound(Entity entity)
{
    Sound& sound = m_Sounds.indexID(entity);
    FMOD_RESULT result = m_pSystem->playSound(sound.pSound, nullptr, false, &sound.pChannel);
    if (result != FMOD_OK) {
        LOG_WARNING("Failed to play sound, entity: %d", entity);
        return false;
    }

    return true;
}
