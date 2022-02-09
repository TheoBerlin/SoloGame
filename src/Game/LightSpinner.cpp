#include "LightSpinner.hpp"

#include <Engine/Transform.hpp>
#include <Engine/Utils/ECSUtils.hpp>

LightSpinner::LightSpinner()
{
    SystemRegistration sysReg = {};
    sysReg.SubscriberRegistration.EntitySubscriptionRegistrations = {
        {
            .pSubscriber = &m_Lights,
            .ComponentAccesses =
            {
                { RW, PositionComponent::Type() }, { NDA, PointLightComponent::Type() }
            }
        }
    };

    RegisterSystem(TYPE_NAME(LightSpinner), sysReg);
}

void LightSpinner::Update(float dt)
{
    const float anglePerSec = DirectX::XM_PIDIV4;
    DirectX::XMVECTOR position;

    ComponentArray<PositionComponent>* pPositionComponents = ECSCore::GetInstance()->GetComponentArray<PositionComponent>();

    for (Entity entity : m_Lights) {
        DirectX::XMFLOAT3& lightPos = pPositionComponents->GetData(entity).Position;

        position = DirectX::XMLoadFloat3(&lightPos);

        position = DirectX::XMVector3Transform(position, DirectX::XMMatrixRotationY(anglePerSec * dt));
        DirectX::XMStoreFloat3(&lightPos, position);
    }
}
