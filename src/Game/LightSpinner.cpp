#include "LightSpinner.hpp"

#include <Engine/Rendering/Components/ComponentGroups.hpp>
#include <Engine/Transform.hpp>
#include <Engine/Utils/ECSUtils.hpp>

LightSpinner::LightSpinner(ECSCore* pECS)
    :System(pECS),
    m_pTransformHandler(nullptr)
{
    PointLightComponents pointLightSub;
    pointLightSub.m_Position.Permissions    = RW;
    pointLightSub.m_PointLight.Permissions  = NDA;

    SystemRegistration sysReg = {};
    sysReg.SubscriberRegistration.EntitySubscriptionRegistrations = {
        {{&pointLightSub}, &m_Lights}
    };
    sysReg.pSystem = this;

    enqueueRegistration(sysReg);
}

LightSpinner::~LightSpinner()
{}

bool LightSpinner::initSystem()
{
    m_pTransformHandler = reinterpret_cast<TransformHandler*>(getComponentHandler(TID(TransformHandler)));
    return m_pTransformHandler;
}

void LightSpinner::update(float dt)
{
    const float anglePerSec = DirectX::XM_PIDIV4;
    DirectX::XMVECTOR position;

    for (Entity entity : m_Lights.getIDs()) {
        DirectX::XMFLOAT3& lightPos = m_pTransformHandler->getPosition(entity);

        position = DirectX::XMLoadFloat3(&lightPos);

        position = DirectX::XMVector3Transform(position, DirectX::XMMatrixRotationY(anglePerSec * dt));
        DirectX::XMStoreFloat3(&lightPos, position);
    }
}
