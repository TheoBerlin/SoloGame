#pragma once

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/ECS/ComponentSubscriptionRequest.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <DirectXMath.h>

const DirectX::XMVECTOR g_DefaultForward = {0.0f, 0.0f, -1.0f};
const DirectX::XMVECTOR g_DefaultUp = {0.0f, 1.0f, 0.0f};

struct Position {
    DirectX::XMFLOAT3 Position;
};

struct Scale {
    DirectX::XMFLOAT3 Scale;
};

struct Rotation {
    DirectX::XMFLOAT4 RotationQuaternion;
};

const std::type_index g_TIDPosition = TID(Position);
const std::type_index g_TIDScale    = TID(Scale);
const std::type_index g_TIDRotation = TID(Rotation);

struct Transform {
    DirectX::XMFLOAT3& Position;
    DirectX::XMFLOAT3& Scale;
    DirectX::XMFLOAT4& RotationQuaternion;
};

class TransformGroup : public IComponentGroup
{
public:
    std::vector<ComponentAccess> toVector() const override final
    {
        return {m_Position, m_Scale, m_Rotation};
    }

public:
    ComponentAccess m_Position    = {R, g_TIDPosition};
    ComponentAccess m_Scale       = {R, g_TIDScale};
    ComponentAccess m_Rotation    = {R, g_TIDRotation};
};

struct WorldMatrix {
    DirectX::XMFLOAT4X4 worldMatrix;
    // Flags whether or not the potential belonging Transform component has been written to since the last access
    bool dirty;
};

const std::type_index g_TIDWorldMatrix = TID(WorldMatrix);

class TransformHandler : public ComponentHandler
{
public:
    TransformHandler(ECSCore* pECs);
    ~TransformHandler();

    virtual bool init() override;

    void createPosition(Entity entity, const DirectX::XMFLOAT3& position = {0.0f, 0.0f, 0.0f});
    void createScale(Entity entity, const DirectX::XMFLOAT3& scale = {1.0f, 1.0f, 1.0f});
    void createRotation(Entity entity);

    void createTransform(Entity entity, const DirectX::XMFLOAT3& position = {0.0f, 0.0f, 0.0f}, const DirectX::XMFLOAT3& scale = {1.0f, 1.0f, 1.0f});
    // Requires that the entity has a transform component
    void createWorldMatrix(Entity entity);

    Transform getTransform(Entity entity);
    // Requires that the entity has both a transform and world matrix component
    WorldMatrix& getWorldMatrix(Entity entity);


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

public:
    IDDVector<Position>     m_Positions;
    IDDVector<Scale>        m_Scales;
    IDDVector<Rotation>     m_Rotations;
    IDDVector<WorldMatrix>  m_WorldMatrices;
};
