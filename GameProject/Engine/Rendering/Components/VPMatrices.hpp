#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <DirectXMath.h>

struct ViewMatrix {
    DirectX::XMFLOAT4X4 view;
};

struct ProjMatrix {
    DirectX::XMFLOAT4X4 projection;
};

const std::type_index tid_view = std::type_index(typeid(ViewMatrix));
const std::type_index tid_projection = std::type_index(typeid(ProjMatrix));

class VPHandler : private ComponentHandler
{
public:
    VPHandler(SystemSubscriber* sysSubscriber);
    ~VPHandler();

    void createViewMatrix(Entity entity, DirectX::XMVECTOR eyePos, DirectX::XMVECTOR lookDir, DirectX::XMVECTOR upDir);
    void createProjMatrix(Entity entity, float horizontalFOV, float aspectRatio, float nearZ, float farZ);

    IDVector<ViewMatrix> viewMatrices;
    IDVector<ProjMatrix> projMatrices;
};
