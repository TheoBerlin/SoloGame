#include "ComponentHandler.hpp"

#include <Engine/ECS/SystemHandler.hpp>

ComponentHandler::ComponentHandler(std::vector<std::type_index> componentTypes, SystemHandler* systemHandler)
{
    this->systemHandler = systemHandler;

    for (size_t i = 0; i < componentTypes.size(); i += 1) {
        systemHandler->registerComponentHandler(componentTypes[i], this);
    }
}

ComponentHandler::~ComponentHandler()
{
    for (size_t i = 0; i < handledTypes.size(); i += 1) {
        systemHandler->deregisterComponentHandler(handledTypes[i]);
    }
}
