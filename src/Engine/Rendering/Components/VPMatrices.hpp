#pragma once

#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <DirectXMath.h>

struct ViewProjectionMatricesComponent {
    DECL_COMPONENT(ViewProjectionMatricesComponent);
    DirectX::XMFLOAT4X4 View;
    DirectX::XMFLOAT4X4 Projection;
};

struct ViewMatrixInfo {
    DirectX::XMVECTOR EyePosition;
    DirectX::XMVECTOR LookDirection;
    DirectX::XMVECTOR UpDirection;
};

struct ProjectionMatrixInfo {
    float HorizontalFOV;
    float AspectRatio;
    float NearZ, FarZ;
};

ViewProjectionMatricesComponent CreateViewProjectionMatrices(const ViewMatrixInfo& viewMatrixInfo, const ProjectionMatrixInfo& projectionMatrixInfo);
