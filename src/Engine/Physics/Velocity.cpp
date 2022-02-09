#include "Velocity.hpp"

#include <Engine/ECS/Job.hpp>
#include <Engine/Transform.hpp>

VelocityHandler::VelocityHandler()
{
    SystemRegistration sysReg = {};
    sysReg.SubscriberRegistration.EntitySubscriptionRegistrations =
    {
        {
            .pSubscriber = &m_MovingObjects,
            .ComponentAccesses =
            {
                { R, VelocityComponent::Type() }, { RW, PositionComponent::Type() }
            }
        }
    };
    sysReg.Phase = LAST_PHASE;

    RegisterSystem(TYPE_NAME(VelocityHandler), sysReg);
}

void VelocityHandler::Update(float dt)
{
    UNREFERENCED_VARIABLE(dt);

    DirectX::XMVECTOR velocity, position;

    ECSCore* pECS = ECSCore::GetInstance();
    const ComponentArray<VelocityComponent>* pVelocityComponents = pECS->GetComponentArray<VelocityComponent>();
    ComponentArray<PositionComponent>* pPositionComponents = pECS->GetComponentArray<PositionComponent>();

    for (Entity entity : m_MovingObjects) {
        velocity = DirectX::XMLoadFloat3(&pVelocityComponents->GetConstData(entity).Velocity);

        DirectX::XMFLOAT3& positionRef = pPositionComponents->GetData(entity).Position;
        position = DirectX::XMLoadFloat3(&positionRef);

        position = DirectX::XMVectorAdd(position, velocity);
        DirectX::XMStoreFloat3(&positionRef, position);
    }
}
