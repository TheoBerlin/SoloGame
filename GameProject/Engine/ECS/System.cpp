#include "System.hpp"

#include <Engine/ECS/SystemHandler.hpp>

System::System(SystemHandler* systemHandler)
{
    this->systemHandler = systemHandler;
}

System::~System()
{}

void System::setID(size_t ID)
{
    this->systemID = ID;
}

ComponentHandler* System::getHandler(std::type_index componentType) const
{
    return this->systemHandler->getComponentHandler(componentType);
}
