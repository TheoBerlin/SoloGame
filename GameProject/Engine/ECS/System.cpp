#include "System.hpp"

#include <Engine/ECS/SystemHandler.hpp>

System::System(SystemHandler* systemHandler)
{
    this->systemHandler = systemHandler;
}

System::~System()
{}

const std::vector<std::type_index>& System::getComponentTypes() const
{
    return componentTypes;
}

void System::subscribeToComponents()
{
    systemHandler->registerSystem(this->componentTypes, this);
}

ComponentHandler* System::getComponentHandler(std::type_index& componentType)
{
    return this->systemHandler->getComponentHandler(componentType);
}
