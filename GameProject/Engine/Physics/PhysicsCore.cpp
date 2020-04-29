#include "PhysicsCore.hpp"

PhysicsCore::PhysicsCore(ECSCore* pECS)
    :m_TransformHandler(pECS),
    m_VelocityHandler(pECS)
{}

PhysicsCore::~PhysicsCore()
{}
