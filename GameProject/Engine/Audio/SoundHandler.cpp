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
        {g_TIDSound, &m_Sounds, [this](Entity entity){ m_Sounds.indexID(entity).pSound->release(); }}
    };
    registerHandler(handlerReg);
}

SoundHandler::~SoundHandler()
{
    m_pSystem->release();
}

bool SoundHandler::initHandler()
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
    registerComponent(entity, g_TIDSound);
    return true;
}

bool SoundHandler::playSound(Entity entity)
{
    Sound& sound = m_Sounds.indexID(entity);
    FMOD_RESULT result = m_pSystem->playSound(sound.pSound, nullptr, false, &sound.pChannel);
    if (result != FMOD_OK) {
        LOG_WARNING("Failed to play sound, entity: %d, error: %s", entity, FMOD_ErrorString(result));
        return false;
    }

    return true;
}

bool SoundHandler::setVolume(Entity entity, float volume)
{
    Sound& sound = m_Sounds.indexID(entity);
    FMOD_RESULT result = sound.pChannel->setVolume(volume);
    if (result != FMOD_OK) {
        LOG_WARNING("Failed to set sound volume, entity: %d, error: %s", entity, FMOD_ErrorString(result));
        return false;
    }

    return true;
}

bool SoundHandler::loopSound(Entity entity)
{
    Sound& sound = m_Sounds.indexID(entity);
    FMOD_RESULT result = sound.pSound->setMode(FMOD_LOOP_NORMAL);
    if (result != FMOD_OK) {
        LOG_WARNING("Failed to set mode FMOD_LOOP_NORMAL, entity: %d, error: %s", entity, FMOD_ErrorString(result));
        return false;
    }

    result = sound.pSound->setLoopCount(-1);
    if (result != FMOD_OK) {
        LOG_WARNING("Failed to set loop count, entity: %d, error: %s", entity, FMOD_ErrorString(result));
        return false;
    }

    return true;
}
