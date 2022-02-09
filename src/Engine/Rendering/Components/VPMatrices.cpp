#include "VPMatrices.hpp"

ViewProjectionMatricesComponent CreateViewProjectionMatrices(const ViewMatrixInfo& viewMatrixInfo, const ProjectionMatrixInfo& projectionMatrixInfo)
{
    ViewProjectionMatricesComponent vpMatrices = {};

    const DirectX::XMVECTOR& eyePos = viewMatrixInfo.EyePosition;
    DirectX::XMStoreFloat4x4(&vpMatrices.View, DirectX::XMMatrixLookAtLH(eyePos, DirectX::XMVectorAdd(eyePos, viewMatrixInfo.LookDirection), viewMatrixInfo.UpDirection));
    DirectX::XMStoreFloat4x4(&vpMatrices.Projection, DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(projectionMatrixInfo.HorizontalFOV), projectionMatrixInfo.AspectRatio,
        projectionMatrixInfo.NearZ, projectionMatrixInfo.FarZ));

    return vpMatrices;
}
