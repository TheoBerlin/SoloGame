#include "Renderer.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/Utils/Logger.hpp>

Renderer::Renderer(ECSCore* pECS, Device* pDevice, RenderingHandler* pRenderingHandler)
    :EntitySubscriber(pECS),
    m_pDevice(pDevice),
    m_pRenderingHandler(pRenderingHandler),
    m_pECS(pECS)
{}

void Renderer::registerRenderer(const EntitySubscriberRegistration& subscriberRegistration)
{
    subscribeToEntities(subscriberRegistration, std::bind(&Renderer::init, this));
}

ComponentHandler* Renderer::getComponentHandler(const std::type_index& handlerType)
{
    return m_pECS->getComponentPublisher()->getComponentHandler(handlerType);
}
