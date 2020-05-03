#include "Renderer.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

Renderer::Renderer(ECSCore* pECS, IDevice* pDevice)
    :m_pECS(pECS),
    m_pDevice(pDevice)
{
    DeviceDX11* pDeviceDX = reinterpret_cast<DeviceDX11*>(pDevice);
    m_pImmediateContext = pDeviceDX->getContext();
}

Renderer::~Renderer()
{
    m_pECS->getComponentSubscriber()->unsubscribeFromComponents(m_ComponentSubscriptionID);
}

void Renderer::registerRenderer(const RendererRegistration& rendererRegistration)
{
    m_pECS->enqueueRendererRegistration(rendererRegistration);
}

ComponentHandler* Renderer::getComponentHandler(const std::type_index& handlerType)
{
    return m_pECS->getComponentSubscriber()->getComponentHandler(handlerType);
}
