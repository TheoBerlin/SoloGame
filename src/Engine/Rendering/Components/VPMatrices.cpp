#include "VPMatrices.hpp"

const float g_DegreesToRadians = DirectX::XM_PI / 180.0f;

VPHandler::VPHandler(ECSCore* pECS)
    :ComponentHandler(pECS, TID(VPHandler))
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    handlerReg.ComponentRegistrations = {
        {g_TIDViewProjectionMatrices, &m_VPMatrices}
    };

    this->registerHandler(handlerReg);
}

bool VPHandler::initHandler()
{
    return true;
}

void VPHandler::createViewProjectionMatrices(Entity entity, const ViewMatrixInfo& viewMatrixInfo, const ProjectionMatrixInfo& projectionMatrixInfo)
{
    ViewProjectionMatrices vpMatrices = {};

    const DirectX::XMVECTOR& eyePos = viewMatrixInfo.EyePosition;
    DirectX::XMStoreFloat4x4(&vpMatrices.View, DirectX::XMMatrixLookAtLH(eyePos, DirectX::XMVectorAdd(eyePos, viewMatrixInfo.LookDirection), viewMatrixInfo.UpDirection));
    DirectX::XMStoreFloat4x4(&vpMatrices.Projection, DirectX::XMMatrixPerspectiveFovLH(projectionMatrixInfo.HorizontalFOV * g_DegreesToRadians, projectionMatrixInfo.AspectRatio,
        projectionMatrixInfo.NearZ, projectionMatrixInfo.FarZ));

    m_VPMatrices.push_back(vpMatrices, entity);
    registerComponent(entity, g_TIDViewProjectionMatrices);
}
