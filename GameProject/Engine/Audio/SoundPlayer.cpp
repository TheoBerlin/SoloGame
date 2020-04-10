#include "SoundPlayer.hpp"

#include <Engine/Audio/SoundHandler.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Transform.hpp>

SoundPlayer::SoundPlayer(ECSCore* pECS)
    :System(pECS),
    m_pLightHandler(nullptr),
    m_pSoundHandler(nullptr),
    m_pTransformHandler(nullptr)
{
    SystemRegistration sysReg = {};
    sysReg.pSystem = this;
    sysReg.SubscriptionRequests = {
        {{{RW, tid_sound}, {R, tid_pointLight}}, &m_Sounds},
        {{{R, tid_transform}, {R, tid_view}, {R, tid_projection}}, &m_Cameras}
    };

    subscribeToComponents(sysReg);
    registerUpdate(sysReg);
}

SoundPlayer::~SoundPlayer()
{}

bool SoundPlayer::init()
{
    m_pSoundHandler = reinterpret_cast<SoundHandler*>(getComponentHandler(TID(SoundHandler)));
    m_pTransformHandler = reinterpret_cast<TransformHandler*>(getComponentHandler(TID(TransformHandler)));
    m_pLightHandler = reinterpret_cast<LightHandler*>(getComponentHandler(TID(LightHandler)));

    return m_pSoundHandler && m_pTransformHandler && m_pLightHandler;
}

void SoundPlayer::update([[maybe_unused]] float dt)
{
    FMOD::System* pSystem = m_pSoundHandler->getSystem();
    pSystem->update();

    if (m_Cameras.size() == 0) {
        return;
    }

    IDVector<Transform>& transforms = m_pTransformHandler->transforms;
    Transform& camTransform = transforms.indexID(m_Cameras[0]);

    DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&camTransform.position);
    DirectX::XMVECTOR camDir = m_pTransformHandler->getForward(camTransform.rotQuat);

    size_t soundCount = m_Sounds.size();
    IDVector<Sound>& sounds = m_pSoundHandler->m_Sounds;

    IDVector<PointLight>& pointLights = m_pLightHandler->pointLights;

    for (size_t soundNr = 0; soundNr < soundCount; soundNr++) {
        Entity soundEntity = m_Sounds[soundNr];
        Sound& sound = sounds.indexID(soundEntity);
        DirectX::XMFLOAT3& soundPosition = pointLights.indexID(soundEntity).position;

        DirectX::XMVECTOR soundPos = DirectX::XMLoadFloat3(&soundPosition);

        // Calculate volume using distance to camera
        DirectX::XMVECTOR camToSound = DirectX::XMVectorSubtract(soundPos, camPos);
        float soundDistance = DirectX::XMVectorGetX(DirectX::XMVector3Length(camToSound));
        float volume = 1.0f / (soundDistance * soundDistance);

        //sound.pChannel->setVolume(volume);

        // Set left-right panning, [-1,1], where -1 is all left, 1 is all right
        // Ignore vertical difference between camera and sound
        camToSound = DirectX::XMVector3Normalize(DirectX::XMVectorSetY(camToSound, camTransform.position.y));
        float angle = m_pTransformHandler->getOrientedAngle(camDir, camToSound, g_DefaultUp);
        float pan = std::sinf(angle);

        sound.pChannel->setPan(pan);
    }
}
