#include "Camera.hpp"

#include <Engine/Rendering/Components/ComponentGroups.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/ECSUtils.hpp>

CameraSystem::CameraSystem(ECSCore* pECS)
    :System(pECS)
{
    CameraComponents cameraComponents;
    cameraComponents.m_Position.Permissions                 = RW;
    cameraComponents.m_Rotation.Permissions                 = RW;
    cameraComponents.m_ViewProjectionMatrices.Permissions   = RW;

    SystemRegistration sysReg = {
        {
            {{}, {&cameraComponents}, &m_Cameras},
        },
        this
    };

    this->subscribeToComponents(sysReg);
    this->registerUpdate(sysReg);
}

CameraSystem::~CameraSystem()
{}

bool CameraSystem::initSystem()
{
    m_pTransformHandler = static_cast<TransformHandler*>(getComponentHandler(TID(TransformHandler)));
    m_pVPHandler = static_cast<VPHandler*>(getComponentHandler(TID(VPHandler)));

    InputHandler* pInputHandler = static_cast<InputHandler*>(getComponentHandler(TID(InputHandler)));
    m_pKeyboardState = pInputHandler->getKeyboardState();
    m_pMouseState = pInputHandler->getMouseState();

    return m_pTransformHandler && m_pVPHandler && pInputHandler;
}

void CameraSystem::update(float dt)
{
    for (Entity entity : m_Cameras.getIDs()) {
        DirectX::XMFLOAT3& position             = m_pTransformHandler->getPosition(entity);
        DirectX::XMFLOAT4& rotationQuaternion   = m_pTransformHandler->getRotation(entity);
        ViewProjectionMatrices& vpMatrices = m_pVPHandler->getViewProjectionMatrices(entity);

        DirectX::XMVECTOR lookDir = m_pTransformHandler->getForward(rotationQuaternion);
        DirectX::XMVECTOR pitchAxis = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(g_DefaultUp, lookDir));

        // React to mouse input
        if (m_pMouseState->x || m_pMouseState->y) {
            DirectX::XMVECTOR rotation = DirectX::XMLoadFloat4(&rotationQuaternion);

            // Limit pitch
            float pitch = m_pTransformHandler->getPitch(lookDir);
            float addedPitch = m_pMouseState->y * dt * 1.3f;
            float newPitch = pitch + addedPitch;

            if (std::abs(newPitch) > maxPitch) {
                addedPitch = newPitch > 0.0f ? maxPitch - pitch : -maxPitch - pitch;
            }

            rotation = DirectX::XMQuaternionMultiply(rotation, DirectX::XMQuaternionRotationAxis(g_DefaultUp, m_pMouseState->x * dt * 1.3f));
            rotation = DirectX::XMQuaternionMultiply(rotation, DirectX::XMQuaternionRotationAxis(pitchAxis, addedPitch));
            DirectX::XMStoreFloat4(&rotationQuaternion, rotation);
            lookDir = m_pTransformHandler->getForward(rotationQuaternion);
        }

        DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&position);

        // React to keyboard input
        if ((m_pKeyboardState->W-m_pKeyboardState->S) || (m_pKeyboardState->D-m_pKeyboardState->A) || (m_pKeyboardState->LeftShift-m_pKeyboardState->LeftControl)) {
            DirectX::XMVECTOR camMove = {0.0f, 0.0f, 0.0f, 0.0f};

            camMove = DirectX::XMVectorAdd(camMove, DirectX::XMVectorScale(lookDir, (float)(m_pKeyboardState->W-m_pKeyboardState->S)));
            camMove = DirectX::XMVectorAdd(camMove, DirectX::XMVectorScale(pitchAxis, (float)(m_pKeyboardState->D-m_pKeyboardState->A)));
            camMove = DirectX::XMVectorAdd(camMove, DirectX::XMVectorScale(g_DefaultUp, (float)(m_pKeyboardState->LeftShift-m_pKeyboardState->LeftControl)));

            camMove = DirectX::XMVectorScale(DirectX::XMVector3Normalize(camMove), dt * g_CameraSpeed);
            camPos = DirectX::XMVectorAdd(camPos, camMove);
            DirectX::XMStoreFloat3(&position, camPos);
        }

        DirectX::XMVECTOR upDir = TransformHandler::getUp(rotationQuaternion);
        DirectX::XMStoreFloat4x4(&vpMatrices.View, DirectX::XMMatrixLookAtLH(camPos, DirectX::XMVectorAdd(camPos, lookDir), upDir));
    }
}
