#include "Camera.hpp"

#include <Engine/Physics/Velocity.hpp>
#include <Engine/Rendering/Components/ComponentGroups.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/ECSUtils.hpp>

CameraSystem::CameraSystem(ECSCore* pECS, InputHandler* pInputHandler)
    :System(pECS),
    m_pTransformHandler(nullptr),
    m_pVelocityHandler(nullptr),
    m_pVPHandler(nullptr),
    m_pInputHandler(pInputHandler)
{
    CameraComponents cameraComponents;
    cameraComponents.m_Position.Permissions                 = R;
    cameraComponents.m_Rotation.Permissions                 = RW;
    cameraComponents.m_ViewProjectionMatrices.Permissions   = RW;
    cameraComponents.m_Velocity.Permissions                 = RW;

    SystemRegistration sysReg = {};
    sysReg.SubscriberRegistration.ComponentSubscriptionRequests = {
        {{&cameraComponents}, &m_Cameras},
    };
    sysReg.pSystem = this;

    enqueueRegistration(sysReg);
}

CameraSystem::~CameraSystem()
{}

bool CameraSystem::initSystem()
{
    m_pTransformHandler = reinterpret_cast<TransformHandler*>(getComponentHandler(TID(TransformHandler)));
    m_pVelocityHandler  = reinterpret_cast<VelocityHandler*>(getComponentHandler(TID(VelocityHandler)));
    m_pVPHandler        = reinterpret_cast<VPHandler*>(getComponentHandler(TID(VPHandler)));

    return m_pTransformHandler && m_pVelocityHandler && m_pVPHandler;
}

void CameraSystem::update(float dt)
{
    const glm::dvec2& mouseMove = m_pInputHandler->getMouseMove();

    for (Entity entity : m_Cameras.getIDs()) {
        DirectX::XMFLOAT4& rotationQuaternion   = m_pTransformHandler->getRotation(entity);
        ViewProjectionMatrices& vpMatrices = m_pVPHandler->getViewProjectionMatrices(entity);

        DirectX::XMVECTOR lookDir = m_pTransformHandler->getForward(rotationQuaternion);
        DirectX::XMVECTOR pitchAxis = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(g_DefaultUp, lookDir));

        // React to mouse input
        if (mouseMove.x != 0.0 || mouseMove.y != 0.0) {
            DirectX::XMVECTOR rotation = DirectX::XMLoadFloat4(&rotationQuaternion);

            // Limit pitch
            float pitch = m_pTransformHandler->getPitch(lookDir);
            float addedPitch = (float)mouseMove.y * dt;
            float newPitch = pitch + addedPitch;

            if (std::abs(newPitch) > maxPitch) {
                addedPitch = newPitch > 0.0f ? maxPitch - pitch : -maxPitch - pitch;
            }

            rotation = DirectX::XMQuaternionMultiply(rotation, DirectX::XMQuaternionRotationAxis(pitchAxis, addedPitch));
            rotation = DirectX::XMQuaternionMultiply(rotation, DirectX::XMQuaternionRotationAxis(g_DefaultUp, (float)mouseMove.x * dt));
            DirectX::XMStoreFloat4(&rotationQuaternion, rotation);
            lookDir = m_pTransformHandler->getForward(rotationQuaternion);
        }

        // React to keyboard input
        DirectX::XMVECTOR camMove = {0.0f, 0.0f, 0.0f, 0.0f};

        int forwardMove = m_pInputHandler->keyState(GLFW_KEY_W) - m_pInputHandler->keyState(GLFW_KEY_S);
        int strafeMove  = m_pInputHandler->keyState(GLFW_KEY_D) - m_pInputHandler->keyState(GLFW_KEY_A);
        int upMove      = m_pInputHandler->keyState(GLFW_KEY_LEFT_SHIFT) - m_pInputHandler->keyState(GLFW_KEY_LEFT_CONTROL);

        if (forwardMove || strafeMove || upMove) {
            camMove = DirectX::XMVectorAdd(camMove, DirectX::XMVectorScale(lookDir, (float)forwardMove));
            camMove = DirectX::XMVectorAdd(camMove, DirectX::XMVectorScale(pitchAxis, (float)strafeMove));
            camMove = DirectX::XMVectorAdd(camMove, DirectX::XMVectorScale(g_DefaultUp, (float)upMove));

            camMove = DirectX::XMVectorScale(DirectX::XMVector3Normalize(camMove), dt * g_CameraSpeed);
        }

        DirectX::XMFLOAT3& cameraVelocity = m_pVelocityHandler->getVelocity(entity);
        DirectX::XMStoreFloat3(&cameraVelocity, DirectX::XMVectorAdd(camMove, DirectX::XMLoadFloat3(&cameraVelocity)));

        DirectX::XMFLOAT3& position = m_pTransformHandler->getPosition(entity);
        DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&position);
        DirectX::XMVECTOR upDir = TransformHandler::getUp(rotationQuaternion);

        DirectX::XMStoreFloat4x4(&vpMatrices.View, DirectX::XMMatrixLookAtLH(camPos, DirectX::XMVectorAdd(camPos, lookDir), upDir));
    }
}
