#include "Camera.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/InputHandler.hpp>
#include <Engine/Transform.hpp>

CameraSystem::CameraSystem(ECSInterface* ecs)
    :System(ecs)
{
    std::type_index tid_transformHandler = std::type_index(typeid(TransformHandler));
    std::type_index tid_vpHandler = std::type_index(typeid(VPHandler));
    std::type_index tid_inputHandler = std::type_index(typeid(InputHandler));

    this->transformHandler = static_cast<TransformHandler*>(ecs->systemSubscriber.getComponentHandler(tid_transformHandler));
    this->vpHandler = static_cast<VPHandler*>(ecs->systemSubscriber.getComponentHandler(tid_vpHandler));

    InputHandler* inputHandler = static_cast<InputHandler*>(ecs->systemSubscriber.getComponentHandler(tid_inputHandler));
    this->keyboardState = inputHandler->getKeyboardState();
    this->mouseState = inputHandler->getMouseState();

    SystemRegistration sysReg = {
    {
        {{{RW, tid_transform}, {RW, tid_view}, {R, tid_projection}}, &cameras},
    },
    this};

    this->subscribeToComponents(&sysReg);
    this->registerUpdate(&sysReg);
}

CameraSystem::~CameraSystem()
{}

void CameraSystem::update(float dt)
{
    for (size_t i = 0; i < cameras.size(); i += 1) {
        Transform& camTransform = transformHandler->transforms.indexID(cameras[i]);
        ViewMatrix& viewMatrix = vpHandler->viewMatrices.indexID(cameras[i]);

        DirectX::XMVECTOR lookDir = transformHandler->getForward(camTransform.rotQuat);
        DirectX::XMVECTOR rightDir = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(defaultUp, lookDir));

        // React to mouse input
        if (mouseState->x || mouseState->y) {
            DirectX::XMVECTOR rotation = DirectX::XMLoadFloat4(&camTransform.rotQuat);

            // Limit pitch
            float pitch = transformHandler->getPitch(lookDir);
            float addedPitch = mouseState->y * dt * 1.3f;
            float newPitch = pitch + addedPitch;

            if (std::abs(newPitch) > maxPitch) {
                addedPitch = newPitch > 0.0f ? maxPitch - pitch : -maxPitch - pitch;
            }

            rotation = DirectX::XMQuaternionMultiply(rotation, DirectX::XMQuaternionRotationAxis(defaultUp, mouseState->x * dt * 1.3f));
            rotation = DirectX::XMQuaternionMultiply(rotation, DirectX::XMQuaternionRotationAxis(rightDir, addedPitch));
            DirectX::XMStoreFloat4(&camTransform.rotQuat, rotation);
            lookDir = transformHandler->getForward(camTransform.rotQuat);
        }

        DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&camTransform.position);

        // React to keyboard input
        if ((keyboardState->W-keyboardState->S) || (keyboardState->D-keyboardState->A) || (keyboardState->LeftShift-keyboardState->LeftControl)) {
            DirectX::XMVECTOR camMove = {0.0f, 0.0f, 0.0f, 0.0f};

            camMove = DirectX::XMVectorAdd(camMove, DirectX::XMVectorScale(lookDir, (float)(keyboardState->W-keyboardState->S)));
            camMove = DirectX::XMVectorAdd(camMove, DirectX::XMVectorScale(rightDir, (float)(keyboardState->D-keyboardState->A)));
            camMove = DirectX::XMVectorAdd(camMove, DirectX::XMVectorScale(defaultUp, (float)(keyboardState->LeftShift-keyboardState->LeftControl)));

            camMove = DirectX::XMVectorScale(DirectX::XMVector3Normalize(camMove), dt * 1.5f);
            camPos = DirectX::XMVectorAdd(camPos, camMove);
            DirectX::XMStoreFloat3(&camTransform.position, camPos);
        }

        DirectX::XMStoreFloat4x4(&viewMatrix.view, DirectX::XMMatrixLookAtLH(camPos, DirectX::XMVectorAdd(camPos, lookDir), {0.0f, 1.0f, 0.0f, 0.0f}));
    }
}
