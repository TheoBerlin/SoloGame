#include "VPMatrices.hpp"

const float degreesToRadians = DirectX::XM_PI / 180.0f;

VPHandler::VPHandler(SystemSubscriber* sysSubscriber)
    :ComponentHandler({tid_view, tid_projection}, sysSubscriber, std::type_index(typeid(VPHandler)))
{}

VPHandler::~VPHandler()
{}

void VPHandler::createViewMatrix(Entity entity, DirectX::XMVECTOR eyePos, DirectX::XMVECTOR lookDir, DirectX::XMVECTOR upDir)
{
    viewMatrices.push_back(ViewMatrix(), entity);
    DirectX::XMStoreFloat4x4(&viewMatrices.back().view, DirectX::XMMatrixLookAtLH(eyePos, DirectX::XMVectorAdd(eyePos, lookDir), upDir));
    this->registerComponent(tid_view, entity);
}

void VPHandler::createProjMatrix(Entity entity, float horizontalFOV, float aspectRatio, float nearZ, float farZ)
{
    projMatrices.push_back(ProjMatrix(), entity);
    DirectX::XMStoreFloat4x4(&projMatrices.back().projection, DirectX::XMMatrixPerspectiveFovLH(horizontalFOV*degreesToRadians, aspectRatio, nearZ, farZ));
    this->registerComponent(tid_projection, entity);
}
