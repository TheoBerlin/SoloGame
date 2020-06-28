#pragma once

#include <Engine/Physics/Velocity.hpp>
#include <Engine/Transform.hpp>

class PhysicsCore
{
public:
    PhysicsCore(ECSCore* pECS);
    ~PhysicsCore();

private:
    TransformHandler m_TransformHandler;
    VelocityHandler m_VelocityHandler;
};
