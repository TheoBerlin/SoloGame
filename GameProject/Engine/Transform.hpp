#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <DirectXMath.h>

const DirectX::XMFLOAT3 defaultForward = {0.0f, 0.0f, -1.0f};
const DirectX::XMFLOAT3 defaultUp = {0.0f, 1.0f, 0.0f};

struct Transform {
    DirectX::XMFLOAT3 position, scale;
    DirectX::XMFLOAT4 rotQuat;
};

struct WorldMatrix {
    DirectX::XMFLOAT4X4 worldMatrix;
    // Flags whether or not the potential belonging Transform component has been written to since the last access
    bool dirty;
};

const std::type_index tid_transform = std::type_index(typeid(Transform));
const std::type_index tid_worldMatrix = std::type_index(typeid(WorldMatrix));

class TransformHandler : public ComponentHandler
{
public:
    TransformHandler(SystemSubscriber* sysSubscriber);
    ~TransformHandler();

    void createTransform(Entity entity);

    // Requires that the entity has both a transform and world matrix component
    WorldMatrix& getWorldMatrix(Entity entity);

    IDVector<Transform> transforms;
    IDVector<WorldMatrix> worldMatrices;
};
