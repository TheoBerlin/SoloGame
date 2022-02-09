#include "SoundPlayer.hpp"

#include <Engine/Rendering/Camera.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Physics/Velocity.hpp>
#include <Engine/Transform.hpp>

#include <fmod_errors.h>

SoundPlayer::SoundPlayer()
    :m_pSystem(nullptr)
{
    SystemRegistration sysReg = {};
    sysReg.SubscriberRegistration.EntitySubscriptionRegistrations =
    {
        {
            .pSubscriber = &m_Sounds,
            .ComponentAccesses =
            {
                { RW, SoundComponent::Type() }, { R, PositionComponent::Type() }
            },
        },
        {
            .pSubscriber = &m_LoopedSounds,
            .ComponentAccesses =
            {
                { RW, SoundLooperComponent::Type() }, { RW, SoundComponent::Type() }
            },
        },
        {
            .pSubscriber = &m_Cameras,
            .ComponentAccesses =
            {
                { R, PositionComponent::Type() }, { R, RotationComponent::Type() }, { R, VelocityComponent::Type() },
                { NDA, CameraTagComponent::Type() }
            }
        }
    };

    RegisterSystem(TYPE_NAME(SoundPlayer), sysReg);
}

SoundPlayer::~SoundPlayer()
{
    m_pSystem->release();
}

bool SoundPlayer::Init()
{
    FMOD_RESULT result = FMOD::System_Create(&m_pSystem);
    if (result != FMOD_OK) {
        LOG_ERRORF("Failed to create FMOD system: %s", FMOD_ErrorString(result));
        return false;
    }

    const int maxChannels = 512;

    result = m_pSystem->init(maxChannels, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK) {
        LOG_ERRORF("Failed to initialize FMOD system: %s", FMOD_ErrorString(result));
        return false;
    }

    return true;
}

void SoundPlayer::Update(float dt)
{
    m_pSystem->update();

    if (m_Cameras.Empty()) {
        return;
    }

    ECSCore* pECS = ECSCore::GetInstance();
    const ComponentArray<PositionComponent>* pPositionComponents = pECS->GetComponentArray<PositionComponent>();
    const ComponentArray<VelocityComponent>* pVelocityComponents = pECS->GetComponentArray<VelocityComponent>();
    ComponentArray<SoundComponent>* pSoundComponents = pECS->GetComponentArray<SoundComponent>();
    ComponentArray<SoundLooperComponent>* pSoundLooperComponents = pECS->GetComponentArray<SoundLooperComponent>();

    const Entity cameraEntity = m_Cameras[0];
    const DirectX::XMFLOAT3& cameraPosition = pPositionComponents->GetConstData(cameraEntity).Position;
    const DirectX::XMFLOAT4& camRotationQuaternion = pECS->GetConstComponent<RotationComponent>(cameraEntity).Quaternion;

    const DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&cameraPosition);
    const DirectX::XMVECTOR camDir = GetForward(camRotationQuaternion);
    DirectX::XMVECTOR camDirFlat = camDir;
    camDirFlat = DirectX::XMVector3Normalize(DirectX::XMVectorSetY(camDirFlat, 0.0f));

    const DirectX::XMVECTOR camVelocity = DirectX::XMLoadFloat3(&pVelocityComponents->GetConstData(cameraEntity).Velocity);
    float camSpeed = DirectX::XMVectorGetX(DirectX::XMVector3Length(camVelocity));

    for (Entity soundEntity : m_Sounds) {
        SoundComponent& sound = pSoundComponents->GetData(soundEntity);
        const DirectX::XMFLOAT3& soundPosition = pPositionComponents->GetConstData(soundEntity).Position;

        DirectX::XMVECTOR soundPos = DirectX::XMLoadFloat3(&soundPosition);

        // Calculate volume using distance to camera
        DirectX::XMVECTOR camToSound = DirectX::XMVectorSubtract(soundPos, camPos);
        float soundDistance = DirectX::XMVectorGetX(DirectX::XMVector3Length(camToSound));
        float volume = 1.0f / soundDistance;

        sound.pChannel->setVolume(volume);

        stereoPan(sound, camToSound, camDirFlat);

        const VelocityComponent* pSoundComponent = nullptr;
        if (pVelocityComponents->GetConstIf(soundEntity, &pSoundComponent)) {
            dopplerEffect(sound, camPos, camVelocity, camSpeed, soundPos, pSoundComponent->Velocity);
        }
    }

    for (Entity loopedSoundEntity : m_LoopedSounds) {
        SoundLooperComponent& soundLooper = pSoundLooperComponents->GetData(loopedSoundEntity);
        soundLooper.NextLoopCountdown -= dt;

        if (soundLooper.NextLoopCountdown < 0.0f) {
            SoundComponent& sound = pSoundComponents->GetData(loopedSoundEntity);
            PlaySound(sound);
            soundLooper.NextLoopCountdown = GetSoundDuration(sound);
        }
    }
}

SoundComponent SoundPlayer::CreateSound(const std::string& fileName)
{
    SoundComponent sound = {};
    const FMOD_RESULT result = m_pSystem->createSound(fileName.c_str(), FMOD_DEFAULT, nullptr, &sound.pSound);
    if (result != FMOD_OK) {
        LOG_WARNINGF("Failed to create sound component from file [%s]: %s", fileName.c_str(), FMOD_ErrorString(result));
    }

    return sound;
}

bool SoundPlayer::PlaySound(SoundComponent& sound)
{
    const FMOD_RESULT result = m_pSystem->playSound(sound.pSound, nullptr, false, &sound.pChannel);
    if (result != FMOD_OK) {
        LOG_WARNINGF("Failed to play sound: %s", FMOD_ErrorString(result));
        return false;
    }

    return true;
}

bool SoundPlayer::SetVolume(SoundComponent& sound, float volume)
{
    const FMOD_RESULT result = sound.pChannel->setVolume(volume);
    if (result != FMOD_OK) {
        LOG_WARNINGF("Failed to set sound volume: %s", FMOD_ErrorString(result));
        return false;
    }

    return true;
}

SoundLooperComponent SoundPlayer::CreateSoundLooper(const SoundComponent& sound)
{
    return SoundLooperComponent{
        .NextLoopCountdown = GetSoundDuration(sound)
    };
}

float SoundPlayer::GetSoundDuration(const SoundComponent& sound)
{
    unsigned int soundDurationMS = 0;
    const FMOD_RESULT result = sound.pSound->getLength(&soundDurationMS, FMOD_TIMEUNIT_MS);
    if (result != FMOD_OK) {
        LOG_WARNINGF("Failed to get sound duration: %s", FMOD_ErrorString(result));
        return 0.0f;
    }

    return float(soundDurationMS) * 0.001f;
}

void SoundPlayer::stereoPan(SoundComponent& sound, const DirectX::XMVECTOR& camToSound, const DirectX::XMVECTOR& camDirFlat)
{
    // Ignore vertical difference between camera and sound, this allows for calculating the yaw
    const DirectX::XMVECTOR camToSoundNorm = DirectX::XMVector3Normalize(DirectX::XMVectorSetY(camToSound, 0.0f));

    const float angle = -GetOrientedAngle(camToSoundNorm, camDirFlat, g_DefaultUp);
    const float cosAngle = std::cosf(angle);
    const float sinAngle = std::sinf(angle);

    const float sqrtTwoRec = 1.0f / std::sqrtf(2.0f);
    const float ampLeft   = 0.5f * (sqrtTwoRec * (cosAngle + sinAngle)) + 0.5f;
    const float ampRight  = 0.5f * (sqrtTwoRec * (cosAngle - sinAngle)) + 0.5f;

    sound.pChannel->setMixLevelsOutput(ampLeft, ampRight, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}

void SoundPlayer::dopplerEffect(SoundComponent& sound, const DirectX::XMVECTOR& camPos, const DirectX::XMVECTOR& camVelocity, float camSpeed, const DirectX::XMVECTOR& objectPos, const DirectX::XMFLOAT3& objectVelocity)
{
    const DirectX::XMVECTOR objVelocity = DirectX::XMLoadFloat3(&objectVelocity);

    // Calculate signs for velocities
    const DirectX::XMVECTOR camToObject = DirectX::XMVectorSubtract(objectPos, camPos);

    // camVelocity is positive if it is moving towards the sound
    const float camVelocitySign = DirectX::XMVectorGetX(DirectX::XMVector3Dot(camVelocity, camToObject));

    // objectVelocity is positive if it is moving away from the receiver
    const float objectVelocitySign = DirectX::XMVectorGetX(DirectX::XMVector3Dot(objVelocity, camToObject));

    camSpeed = camVelocitySign < 0.0f ? -camSpeed : camSpeed;
    float objSpeed = DirectX::XMVectorGetX(DirectX::XMVector3Length(objVelocity));
    objSpeed = objectVelocitySign < 0.0f ? -objSpeed : objSpeed;

    const float soundPropagation = 343.0f;
    float frequency = 0.0f;
    sound.pChannel->getFrequency(&frequency);

    const float observedFrequency = frequency * (soundPropagation + camSpeed) / (soundPropagation + objSpeed);

    sound.pChannel->setFrequency(observedFrequency);
}
