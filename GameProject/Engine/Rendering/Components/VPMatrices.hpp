#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <DirectXMath.h>

struct ViewMatrix {
    DirectX::XMFLOAT4X4 view;
};

struct ProjectionMatrix {
    DirectX::XMFLOAT4X4 projection;
};

const std::type_index tid_view = TID(ViewMatrix);
const std::type_index tid_projection = TID(ProjectionMatrix);

class VPHandler : public ComponentHandler
{
public:
    VPHandler(ECSCore* pECS);
    ~VPHandler();

    virtual bool init() override;

    void createViewMatrix(Entity entity, DirectX::XMVECTOR eyePos, DirectX::XMVECTOR lookDir, DirectX::XMVECTOR upDir);
    void createProjMatrix(Entity entity, float horizontalFOV, float aspectRatio, float nearZ, float farZ);

    IDDVector<ViewMatrix> viewMatrices;
    IDDVector<ProjectionMatrix> projMatrices;
};
