#include "ComponentHandler.hpp"

#include <Engine/ECS/SystemSubscriber.hpp>

ComponentHandler::ComponentHandler(std::vector<std::type_index> handledTypes, SystemSubscriber* systemSubscriber)
    :handledTypes(handledTypes),
    systemSubscriber(systemSubscriber),
    tid_handler(std::type_index(typeid(this)))
{}

ComponentHandler::~ComponentHandler()
{
    systemSubscriber->deregisterComponents(this);
}

void ComponentHandler::registerHandler(std::vector<ComponentRegistration>* componentQueries)
{
    this->systemSubscriber->registerComponents(this, componentQueries);
}

const std::vector<std::type_index>& ComponentHandler::getHandledTypes() const
{
    return this->handledTypes;
}

std::type_index ComponentHandler::getHandlerType() const
{
    return this->tid_handler;
}

void ComponentHandler::registerComponent(std::type_index tid, Entity entityID)
{
    this->systemSubscriber->newComponent(entityID, tid);
}
