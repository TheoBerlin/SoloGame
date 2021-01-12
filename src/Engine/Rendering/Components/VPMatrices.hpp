#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <DirectXMath.h>

struct ViewProjectionMatrices {
    DirectX::XMFLOAT4X4 View;
    DirectX::XMFLOAT4X4 Projection;
};

const std::type_index g_TIDViewProjectionMatrices = TID(ViewProjectionMatrices);

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

class VPHandler : public ComponentHandler
{
public:
    VPHandler(ECSCore* pECS);
    ~VPHandler() = default;

    virtual bool initHandler() override;

    inline ViewProjectionMatrices& getViewProjectionMatrices(Entity entity) { return m_VPMatrices.indexID(entity); }

    void createViewProjectionMatrices(Entity entity, const ViewMatrixInfo& viewMatrixInfo, const ProjectionMatrixInfo& projectionMatrixInfo);

private:
    IDDVector<ViewProjectionMatrices> m_VPMatrices;
};
