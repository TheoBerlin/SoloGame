#include "SoundPlayer.hpp"

#include <Engine/Audio/SoundHandler.hpp>
#include <Engine/Rendering/Components/ComponentGroups.hpp>
#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Transform.hpp>

SoundPlayer::SoundPlayer(ECSCore* pECS)
    :System(pECS),
    m_pLightHandler(nullptr),
    m_pSoundHandler(nullptr),
    m_pTransformHandler(nullptr)
{
    CameraComponents cameraComponents;

    SystemRegistration sysReg = {};
    sysReg.pSystem = this;
    sysReg.SubscriptionRequests = {
        {{{RW, tid_sound}, {R, tid_pointLight}}, &m_Sounds},
        {{&cameraComponents}, &m_Cameras}
    };

    subscribeToComponents(sysReg);
    registerUpdate(sysReg);
}

SoundPlayer::~SoundPlayer()
{}

bool SoundPlayer::init()
{
    m_pSoundHandler     = reinterpret_cast<SoundHandler*>(getComponentHandler(TID(SoundHandler)));
    m_pTransformHandler = reinterpret_cast<TransformHandler*>(getComponentHandler(TID(TransformHandler)));
    m_pLightHandler     = reinterpret_cast<LightHandler*>(getComponentHandler(TID(LightHandler)));

    return m_pSoundHandler && m_pTransformHandler && m_pLightHandler;
}

void SoundPlayer::update([[maybe_unused]] float dt)
{
    FMOD::System* pSystem = m_pSoundHandler->getSystem();
    pSystem->update();

    if (m_Cameras.empty()) {
        return;
    }

    const DirectX::XMFLOAT3& cameraPosition = m_pTransformHandler->getPosition(m_Cameras[0]);
    const DirectX::XMFLOAT4& camRotationQuaternion = m_pTransformHandler->getRotation(m_Cameras[0]);

    DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&cameraPosition);
    DirectX::XMVECTOR camDir = m_pTransformHandler->getForward(camRotationQuaternion);
    DirectX::XMVECTOR camDirFlat = camDir;
    camDirFlat = DirectX::XMVector3Normalize(DirectX::XMVectorSetY(camDirFlat, 0.0f));

    float sqrtTwoRec = 1.0f / std::sqrtf(2.0f);

    size_t soundCount = m_Sounds.size();
    IDDVector<Sound>& sounds = m_pSoundHandler->m_Sounds;

    IDDVector<PointLight>& pointLights = m_pLightHandler->pointLights;

    for (Entity soundEntity : m_Sounds.getIDs()) {
        Sound& sound = sounds.indexID(soundEntity);
        DirectX::XMFLOAT3& soundPosition = pointLights.indexID(soundEntity).position;

        DirectX::XMVECTOR soundPos = DirectX::XMLoadFloat3(&soundPosition);

        // Calculate volume using distance to camera
        DirectX::XMVECTOR camToSound = DirectX::XMVectorSubtract(soundPos, camPos);
        float soundDistance = DirectX::XMVectorGetX(DirectX::XMVector3Length(camToSound));
        float volume = 1.0f / soundDistance;

        sound.pChannel->setVolume(volume);

        // Set left-right panning, [-1,1], where -1 is all left, 1 is all right
        // Ignore vertical difference between camera and sound
        camToSound = DirectX::XMVector3Normalize(DirectX::XMVectorSetY(camToSound, 0.0f));

        float angle = -m_pTransformHandler->getOrientedAngle(camToSound, camDirFlat, g_DefaultUp);
        float cosAngle = std::cosf(angle);
        float sinAngle = std::sinf(angle);

        float ampLeft   = 0.5f * (sqrtTwoRec * (cosAngle + sinAngle)) + 0.5f;
        float ampRight  = 0.5f * (sqrtTwoRec * (cosAngle - sinAngle)) + 0.5f;

        sound.pChannel->setMixLevelsOutput(ampLeft, ampRight, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    }
}
