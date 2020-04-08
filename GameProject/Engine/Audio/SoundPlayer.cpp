#include "SoundPlayer.hpp"

#include <Engine/Audio/SoundHandler.hpp>

SoundPlayer::SoundPlayer(ECSCore* pECS)
    :System(pECS),
    m_pSoundHandler(nullptr)
{
    SystemRegistration sysReg = {};
    sysReg.pSystem = this;
    sysReg.SubscriptionRequests = {
        {{{R, tid_sound}}, &m_Sounds}
    };

    subscribeToComponents(sysReg);
    registerUpdate(sysReg);
}

SoundPlayer::~SoundPlayer()
{}

bool SoundPlayer::init()
{
    m_pSoundHandler = reinterpret_cast<SoundHandler*>(getComponentHandler(TID(SoundHandler)));
    return m_pSoundHandler;
}

void SoundPlayer::update([[maybe_unused]] float dt)
{
    FMOD::System* pSystem = m_pSoundHandler->getSystem();
    pSystem->update();
}
