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

        // React to keyboard input
        DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&camTransform.position);

        // Forwards and backwards
        DirectX::XMVECTOR lookDir = transformHandler->getForward(camTransform);
        camPos = DirectX::XMVectorAdd(camPos, DirectX::XMVectorScale(lookDir, (keyboardState->W-keyboardState->S) * dt));

        // Right and left
        DirectX::XMVECTOR rightDir = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(defaultUp, lookDir));
        camPos = DirectX::XMVectorAdd(camPos, DirectX::XMVectorScale(rightDir, (keyboardState->D-keyboardState->A) * dt));

        // Up and down
        camPos = DirectX::XMVectorAdd(camPos, DirectX::XMVectorScale(defaultUp, (keyboardState->LeftShift-keyboardState->LeftControl) * dt));
        DirectX::XMStoreFloat3(&camTransform.position, camPos);

        DirectX::XMStoreFloat4x4(&viewMatrix.view, DirectX::XMMatrixLookAtLH(camPos, DirectX::XMVectorAdd(camPos, lookDir), {0.0f, 1.0f, 0.0f, 0.0f}));
    }
}
