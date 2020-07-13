#include "Renderer.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/DeviceDX11.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

Renderer::Renderer(ECSCore* pECS, Device* pDevice, RenderingHandler* pRenderingHandler)
    :m_pECS(pECS),
    m_pDevice(pDevice),
    m_pRenderingHandler(pRenderingHandler)
{}

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
