#pragma once

#define NOMINMAX

#include <Engine/ECS/ComponentHandler.hpp>
#include <Engine/ECS/System.hpp>
#include <Engine/Utils/ECSUtils.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <DirectXMath.h>

struct Velocity {
    DirectX::XMFLOAT3 Velocity;
};

const std::type_index g_TIDVelocity = TID(Velocity);

class TransformHandler;

class VelocityHandler : public ComponentHandler, public System
{
public:
    VelocityHandler(ECSCore* pECS);
    ~VelocityHandler() = default;

    virtual bool initHandler() override final { return true; };
    virtual bool initSystem() override final;
    virtual void update(float dt) override final;

    void createVelocityComponent(Entity entity, const DirectX::XMFLOAT3& velocity = {0.0f, 0.0f, 0.0f});

    inline bool hasVelocity(Entity entity) { return m_Velocities.hasElement(entity); }
    inline DirectX::XMFLOAT3& getVelocity(Entity entity) { return m_Velocities.indexID(entity).Velocity; }

private:
    IDDVector<Velocity> m_Velocities;

    IDVector m_MovingObjects;

    TransformHandler* m_pTransformHandler;
};
