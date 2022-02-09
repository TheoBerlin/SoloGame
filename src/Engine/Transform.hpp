#pragma once

#include <Engine/ECS/Component.hpp>
#include <Engine/ECS/EntitySubscriber.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <DirectXMath.h>

const DirectX::XMVECTOR g_DefaultForward = {0.0f, 0.0f, -1.0f};
const DirectX::XMVECTOR g_DefaultUp = {0.0f, 1.0f, 0.0f};
constexpr const DirectX::XMFLOAT4 g_QuaternionIdentity = { 0.0f, 0.0f, 0.0f, 1.0f };

struct PositionComponent {
    DECL_COMPONENT(PositionComponent);
    DirectX::XMFLOAT3 Position;
};

struct ScaleComponent {
    DECL_COMPONENT(ScaleComponent);
    DirectX::XMFLOAT3 Scale;
};

struct RotationComponent {
    DECL_COMPONENT(RotationComponent);
    DirectX::XMFLOAT4 Quaternion;
};

class TransformComponents : public IComponentGroup
{
public:
    std::vector<ComponentAccess> ToArray() const override final
    {
        return { m_Position, m_Scale, m_Rotation };
    }

public:
    GroupedComponent<PositionComponent> m_Position;
    GroupedComponent<ScaleComponent> m_Scale;
    GroupedComponent<RotationComponent> m_Rotation;
};

struct WorldMatrixComponent {
    DECL_COMPONENT_WITH_DIRTY_FLAG(WorldMatrixComponent);
    DirectX::XMFLOAT4X4 WorldMatrix;
};

DirectX::XMFLOAT4X4 CreateWorldMatrix(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT4& rotationQuat);

// Transform calculation functions
DirectX::XMVECTOR GetUp(const DirectX::XMFLOAT4& rotationQuat);
DirectX::XMVECTOR GetForward(const DirectX::XMFLOAT4& rotationQuat);
DirectX::XMFLOAT4 GetRotationQuaternion(const DirectX::XMFLOAT3& forward);

float GetPitch(const DirectX::XMVECTOR& forward);
float GetYaw(const DirectX::XMVECTOR& forward);
float GetRoll(const DirectX::XMFLOAT4& rotationQuat);
// Vectors are assumed to be normalized
float GetOrientedAngle(const DirectX::XMVECTOR& V1, const DirectX::XMVECTOR& V2, const DirectX::XMVECTOR& normal);

// Assumes the forward is normalized
void SetForward(DirectX::XMFLOAT4& rotationQuat, const DirectX::XMVECTOR& forward);

void Roll(DirectX::XMFLOAT4& rotationQuat, float angle);

// Rotate V around P using a given axis and angle
void RotateAroundPoint(const DirectX::XMVECTOR& P, DirectX::XMVECTOR& V, const DirectX::XMVECTOR& axis, float angle);
