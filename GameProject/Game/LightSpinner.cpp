#include "LightSpinner.hpp"

#include <Engine/Rendering/Components/PointLight.hpp>
#include <Engine/Utils/ECSUtils.hpp>

LightSpinner::LightSpinner(ECSCore* pECS)
    :System(pECS)
{
    SystemRegistration sysReg = {
    {
        {{{RW, tid_pointLight}}, &m_Lights}
    },
    this};

    subscribeToComponents(sysReg);
    registerUpdate(sysReg);
}

LightSpinner::~LightSpinner()
{}

bool LightSpinner::initSystem()
{
    m_pLightHandler = static_cast<LightHandler*>(getComponentHandler(TID(LightHandler)));
    return m_pLightHandler;
}

void LightSpinner::update(float dt)
{
    size_t lightCount = m_Lights.size();

    const float anglePerSec = DirectX::XM_PIDIV4;
    DirectX::XMVECTOR position;

    for (size_t i = 0; i < lightCount; i += 1) {
        PointLight& light = m_pLightHandler->pointLights.indexID(m_Lights[i]);

        position = DirectX::XMLoadFloat3(&light.position);

        position = DirectX::XMVector3Transform(position, DirectX::XMMatrixRotationY(anglePerSec * dt));
        DirectX::XMStoreFloat3(&light.position, position);
    }
}
