#include "Camera.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/Rendering/Components/VPMatrices.hpp>
#include <Engine/Transform.hpp>

CameraSystem::CameraSystem(ECSInterface* ecs)
    :System(ecs)
{
    std::type_index tid_transformHandler = std::type_index(typeid(TransformHandler));
    std::type_index tid_vpHandler = std::type_index(typeid(VPHandler));

    this->transformHandler = static_cast<TransformHandler*>(ecs->systemSubscriber.getComponentHandler(tid_transformHandler));
    this->vpHandler = static_cast<VPHandler*>(ecs->systemSubscriber.getComponentHandler(tid_vpHandler));

    SystemRegistration sysReg = {
    {
        {{{R, tid_transform}, {RW, tid_view}, {R, tid_projection}}, &cameras},
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

        DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&camTransform.position);
        DirectX::XMVECTOR lookDir = transformHandler->getForward(camTransform);
        DirectX::XMStoreFloat4x4(&viewMatrix.view, DirectX::XMMatrixLookAtLH(camPos, DirectX::XMVectorAdd(camPos, lookDir), {0.0f, 1.0f, 0.0f, 0.0f}));
    }
}
