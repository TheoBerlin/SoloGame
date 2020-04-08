#include "ComponentHandler.hpp"

#include <Engine/ECS/ECSCore.hpp>

ComponentHandler::ComponentHandler(std::vector<std::type_index> handledTypes, ECSCore* pECS, std::type_index tid_handler)
    :m_HandledTypes(handledTypes),
    m_pECS(pECS),
    m_TID(tid_handler)
{}

ComponentHandler::~ComponentHandler()
{
    m_pECS->getComponentSubscriber()->deregisterComponentHandler(this);
}

void ComponentHandler::registerHandler(const ComponentHandlerRegistration& handlerRegistration)
{
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
