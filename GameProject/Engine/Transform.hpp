#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <DirectXMath.h>

const DirectX::XMVECTOR g_DefaultForward = {0.0f, 0.0f, -1.0f};
const DirectX::XMVECTOR g_DefaultUp = {0.0f, 1.0f, 0.0f};

struct Transform {
    DirectX::XMFLOAT3 position, scale;
    DirectX::XMFLOAT4 rotQuat;
};

struct WorldMatrix {
    DirectX::XMFLOAT4X4 worldMatrix;
    // Flags whether or not the potential belonging Transform component has been written to since the last access
    bool dirty;
};

const std::type_index tid_transform = TID(Transform);
const std::type_index tid_worldMatrix = TID(WorldMatrix);

class TransformHandler : public ComponentHandler
{
public:
    TransformHandler(ECSCore* pECs);
    ~TransformHandler();

    virtual bool init() override;

    void createTransform(Entity entity);
    // Requires that the entity has a transform component
    void createWorldMatrix(Entity entity);

    // Requires that the entity has both a transform and world matrix component
    WorldMatrix& getWorldMatrix(Entity entity);

    IDDVector<Transform> transforms;
    IDDVector<WorldMatrix> worldMatrices;

    // Transform calculation functions
    static DirectX::XMVECTOR getUp(const DirectX::XMFLOAT4& rotationQuat);
    static DirectX::XMVECTOR getForward(const DirectX::XMFLOAT4& rotationQuat);
    static DirectX::XMFLOAT4 getRotationQuaternion(const DirectX::XMFLOAT3& forward);

    static float getPitch(const DirectX::XMVECTOR& forward);
    static float getYaw(const DirectX::XMVECTOR& forward);
    static float getRoll(const DirectX::XMFLOAT4& rotationQuat);
    // Vectors are assumed to be normalized
    static float getOrientedAngle(const DirectX::XMVECTOR& V1, const DirectX::XMVECTOR& V2, const DirectX::XMVECTOR& normal);

    // Assumes the forward is normalized
    static void setForward(DirectX::XMFLOAT4& rotationQuat, const DirectX::XMVECTOR& forward);

    static void roll(DirectX::XMFLOAT4& rotationQuat, float angle);

    // Rotate V around P using a given axis and angle
    static void rotateAroundPoint(const DirectX::XMVECTOR& P, DirectX::XMVECTOR& V, const DirectX::XMVECTOR& axis, float angle);
};
