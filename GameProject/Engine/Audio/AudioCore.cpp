#include "AudioCore.hpp"

AudioCore::AudioCore(ECSCore* pECS)
    :m_SoundHandler(pECS),
    m_SoundPlayer(pECS)
{}

AudioCore::~AudioCore()
{}
