#include "Renderer.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/ECS/EntityPublisher.hpp>

Renderer::Renderer(Device* pDevice, RenderingHandler* pRenderingHandler)
    :m_pDevice(pDevice),
    m_pRenderingHandler(pRenderingHandler)
{}

void Renderer::RegisterRenderer(EntitySubscriberRegistration& subscriberRegistration)
{
    SubscribeToEntities(subscriberRegistration);
}
