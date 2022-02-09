#include "Camera.hpp"

#include <Engine/Physics/Velocity.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/ECSUtils.hpp>

CameraSystem::CameraSystem(InputHandler* pInputHandler)
    : m_pInputHandler(pInputHandler)
{
    SystemRegistration sysReg = {};
    sysReg.SubscriberRegistration.EntitySubscriptionRegistrations = {
        {
            .pSubscriber = &m_Cameras,
            .ComponentAccesses =
            {
                { NDA, CameraTagComponent::Type() }, { R, PositionComponent::Type() },
                { RW, RotationComponent::Type() }, { RW, ViewProjectionMatricesComponent::Type()},
                { RW, VelocityComponent::Type() }
            }
        }
    };

    sysReg.Phase = 0u;

    RegisterSystem(TYPE_NAME(CameraSystem), sysReg);
}

void CameraSystem::Update(float dt)
{
    ECSCore* pECS = ECSCore::GetInstance();
    const ComponentArray<PositionComponent>* pPositionComponents = pECS->GetComponentArray<PositionComponent>();
    ComponentArray<VelocityComponent>* pVelocityComponents = pECS->GetComponentArray<VelocityComponent>();
    ComponentArray<RotationComponent>* pRotationComponents = pECS->GetComponentArray<RotationComponent>();
    ComponentArray<ViewProjectionMatricesComponent>* pVPComponents = pECS->GetComponentArray<ViewProjectionMatricesComponent>();

    const glm::dvec2& mouseMove = m_pInputHandler->getMouseMove();

    for (Entity cameraEntity : m_Cameras.GetIDs()) {
        DirectX::XMFLOAT4& rotationQuaternion = pRotationComponents->GetData(cameraEntity).Quaternion;
        ViewProjectionMatricesComponent& vpMatrices = pVPComponents->GetData(cameraEntity);

        DirectX::XMVECTOR lookDir = GetForward(rotationQuaternion);
        DirectX::XMVECTOR pitchAxis = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(g_DefaultUp, lookDir));

        // React to mouse input
        if (mouseMove.x != 0.0 || mouseMove.y != 0.0) {
            DirectX::XMVECTOR rotation = DirectX::XMLoadFloat4(&rotationQuaternion);

            // Limit pitch
            const float pitch = GetPitch(lookDir);
            float addedPitch = (float)mouseMove.y * dt;
            const float newPitch = pitch + addedPitch;

            if (std::abs(newPitch) > g_MaxPitch) {
                addedPitch = newPitch > 0.0f ? g_MaxPitch - pitch : -g_MaxPitch - pitch;
            }

            rotation = DirectX::XMQuaternionMultiply(rotation, DirectX::XMQuaternionRotationAxis(pitchAxis, addedPitch));
            rotation = DirectX::XMQuaternionMultiply(rotation, DirectX::XMQuaternionRotationAxis(g_DefaultUp, (float)mouseMove.x * dt));
            DirectX::XMStoreFloat4(&rotationQuaternion, rotation);
            lookDir = GetForward(rotationQuaternion);
        }

        // React to keyboard input
        DirectX::XMVECTOR camMove = { 0.0f, 0.0f, 0.0f, 0.0f };

        const int forwardMove = m_pInputHandler->KeyState(GLFW_KEY_W) - m_pInputHandler->KeyState(GLFW_KEY_S);
        const int strafeMove  = m_pInputHandler->KeyState(GLFW_KEY_D) - m_pInputHandler->KeyState(GLFW_KEY_A);
        const int upMove      = m_pInputHandler->KeyState(GLFW_KEY_LEFT_SHIFT) - m_pInputHandler->KeyState(GLFW_KEY_LEFT_CONTROL);

        if (forwardMove || strafeMove || upMove) {
            camMove = DirectX::XMVectorAdd(camMove, DirectX::XMVectorScale(lookDir, (float)forwardMove));
            camMove = DirectX::XMVectorAdd(camMove, DirectX::XMVectorScale(pitchAxis, (float)strafeMove));
            camMove = DirectX::XMVectorAdd(camMove, DirectX::XMVectorScale(g_DefaultUp, (float)upMove));

            camMove = DirectX::XMVectorScale(DirectX::XMVector3Normalize(camMove), dt * g_CameraSpeed);
        }

        DirectX::XMFLOAT3& cameraVelocity = pVelocityComponents->GetData(cameraEntity).Velocity;
        DirectX::XMStoreFloat3(&cameraVelocity, camMove);

        const DirectX::XMFLOAT3& position = pPositionComponents->GetConstData(cameraEntity).Position;
        DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&position);
        DirectX::XMVECTOR upDir = GetUp(rotationQuaternion);

        DirectX::XMStoreFloat4x4(&vpMatrices.View, DirectX::XMMatrixLookAtLH(camPos, DirectX::XMVectorAdd(camPos, lookDir), upDir));
    }
}
