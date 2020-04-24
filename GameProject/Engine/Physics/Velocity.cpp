#include "Velocity.hpp"

#include <Engine/ECS/SystemUpdater.hpp>
#include <Engine/Transform.hpp>

VelocityHandler::VelocityHandler(ECSCore* pECS)
    :ComponentHandler(pECS, TID(VelocityHandler)),
    System(pECS),
    m_pTransformHandler(nullptr)
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    handlerReg.ComponentRegistrations = {
        {g_TIDVelocity,     &m_Velocities}
    };

    this->registerHandler(handlerReg);

    SystemRegistration sysReg = {
        {
            {{{RW, g_TIDVelocity}, {RW, g_TIDPosition}}, &m_MovingObjects}
        },
        this,
        g_LastUpdateQueue
    };

    subscribeToComponents(sysReg);
    registerUpdate(sysReg);
}

VelocityHandler::~VelocityHandler()
{}

bool VelocityHandler::initSystem()
{
    m_pTransformHandler = reinterpret_cast<TransformHandler*>(getComponentHandler(TID(TransformHandler)));

    return m_pTransformHandler;
}

void VelocityHandler::update(float dt)
{
    DirectX::XMVECTOR velocity, position;

    for (Entity entity : m_MovingObjects.getIDs()) {
        velocity = DirectX::XMLoadFloat3(&m_Velocities.indexID(entity).Velocity);

        DirectX::XMFLOAT3& positionRef = m_pTransformHandler->getPosition(entity);
        position = DirectX::XMLoadFloat3(&positionRef);

        position = DirectX::XMVectorAdd(position, velocity);
        DirectX::XMStoreFloat3(&positionRef, position);
    }
}

void VelocityHandler::createVelocityComponent(Entity entity, const DirectX::XMFLOAT3& velocity)
{
    m_Velocities.push_back({velocity}, entity);
    registerComponent(entity, g_TIDVelocity);
}
