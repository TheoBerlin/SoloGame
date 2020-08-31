#include "ComponentHandler.hpp"

#include <Engine/ECS/ECSCore.hpp>

ComponentHandler::ComponentHandler(ECSCore* pECS, std::type_index tid_handler)
    :m_pECS(pECS),
    m_TID(tid_handler)
{}

ComponentHandler::~ComponentHandler()
{
    m_pECS->getComponentPublisher()->deregisterComponentHandler(this);
}

void ComponentHandler::registerHandler(const ComponentHandlerRegistration& handlerRegistration)
{
    // Write handled types
    m_HandledTypes.reserve(handlerRegistration.ComponentRegistrations.size());

    for (const ComponentRegistration& componentRegistration : handlerRegistration.ComponentRegistrations) {
        m_HandledTypes.push_back(componentRegistration.tid);
    }

    m_pECS->enqueueComponentHandlerRegistration(handlerRegistration);
}

const std::vector<std::type_index>& ComponentHandler::getHandledTypes() const
{
    return m_HandledTypes;
}

std::type_index ComponentHandler::getHandlerType() const
{
    return m_TID;
}

void ComponentHandler::registerComponent(Entity entity, std::type_index componentType)
{
    m_pECS->componentAdded(entity, componentType);
}
