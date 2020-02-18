#include "LightSpinner.hpp"

#include <Engine/Rendering/Components/PointLight.hpp>

LightSpinner::LightSpinner(ECSCore* pECS)
    :System(pECS)
{
    std::type_index tid_lightHandler = std::type_index(typeid(LightHandler));
    this->lightHandler = static_cast<LightHandler*>(getComponentHandler(tid_lightHandler));

    SystemRegistration sysReg = {
    {
        {{{RW, tid_pointLight}}, &lights}
    },
    this};

    this->subscribeToComponents(&sysReg);
    this->registerUpdate(&sysReg);
}

LightSpinner::~LightSpinner()
{}

void LightSpinner::update(float dt)
{
    size_t lightCount = lights.size();

    const float anglePerSec = DirectX::XM_PIDIV4;
    DirectX::XMVECTOR position;

    for (size_t i = 0; i < lightCount; i += 1) {
        PointLight& light = lightHandler->pointLights.indexID(lights[i]);

        position = DirectX::XMLoadFloat3(&light.position);

        position = DirectX::XMVector3Transform(position, DirectX::XMMatrixRotationY(anglePerSec * dt));
        DirectX::XMStoreFloat3(&light.position, position);
    }
}
