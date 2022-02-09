#pragma once

#include <Engine/Physics/Velocity.hpp>

class PhysicsCore
{
public:
    PhysicsCore() = default;
    ~PhysicsCore() = default;

private:
    VelocityHandler m_VelocityHandler;
};
