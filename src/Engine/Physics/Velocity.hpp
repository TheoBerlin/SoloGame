#pragma once

#include <Engine/ECS/System.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <DirectXMath.h>

struct VelocityComponent {
    DECL_COMPONENT(VelocityComponent);
    DirectX::XMFLOAT3 Velocity;
};

class VelocityHandler : public System
{
public:
    VelocityHandler();
    ~VelocityHandler() = default;

    virtual void Update(float dt) override final;

private:
    IDVector m_MovingObjects;
};
