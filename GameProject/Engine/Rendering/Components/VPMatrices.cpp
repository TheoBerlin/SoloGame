#include "VPMatrices.hpp"

const float degreesToRadians = DirectX::XM_PI / 180.0f;

VPHandler::VPHandler(ECSCore* pECS)
    :ComponentHandler({tid_view, tid_projection}, pECS, std::type_index(typeid(VPHandler)))
{
    std::vector<ComponentRegistration> compRegs = {
        {tid_view, &viewMatrices},
        {tid_projection, &projMatrices}
    };

    this->registerHandler(&compRegs);
}

VPHandler::~VPHandler()
{}

void VPHandler::createViewMatrix(Entity entity, DirectX::XMVECTOR eyePos, DirectX::XMVECTOR lookDir, DirectX::XMVECTOR upDir)
{
    ViewMatrix viewMatrix;
    DirectX::XMStoreFloat4x4(&viewMatrix.view, DirectX::XMMatrixLookAtLH(eyePos, DirectX::XMVectorAdd(eyePos, lookDir), upDir));
    viewMatrices.push_back(viewMatrix, entity);
    this->registerComponent(entity, tid_view);
}

void VPHandler::createProjMatrix(Entity entity, float horizontalFOV, float aspectRatio, float nearZ, float farZ)
{
    ProjectionMatrix projMatrix;
    DirectX::XMStoreFloat4x4(&projMatrix.projection, DirectX::XMMatrixPerspectiveFovLH(horizontalFOV*degreesToRadians, aspectRatio, nearZ, farZ));
    projMatrices.push_back(projMatrix, entity);
    this->registerComponent(entity, tid_projection);
}
