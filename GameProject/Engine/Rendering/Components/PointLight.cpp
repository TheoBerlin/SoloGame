#include "PointLight.hpp"

LightHandler::LightHandler(ECSCore* pECS)
    :ComponentHandler(pECS, TID(LightHandler))
{
    ComponentHandlerRegistration handlerReg = {};
    handlerReg.pComponentHandler = this;
    handlerReg.ComponentRegistrations = {
        {g_TIDPointLight, &m_PointLights}
    };

    this->registerHandler(handlerReg);
}

LightHandler::~LightHandler()
{}

bool LightHandler::initHandler()
{
    return true;
}

void LightHandler::createPointLight(Entity entity, const DirectX::XMFLOAT3& light, float radius)
{
    PointLight pointLight;
    pointLight.Light            = light;
    pointLight.RadiusReciprocal = 1.0f / radius;
    m_PointLights.push_back(pointLight, entity);
    this->registerComponent(entity, g_TIDPointLight);
}
