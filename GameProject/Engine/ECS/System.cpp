#include "System.hpp"

#include <Engine/ECS/ECSInterface.hpp>
#include <Engine/ECS/SystemSubscriber.hpp>

System::System(ECSInterface* ecs)
    :ecs(ecs)
{}

System::~System()
{}

void System::subscribeToComponents(SystemRegistration* sysReg)
{
    ecs->systemSubscriber.registerSystem(sysReg);
}

void System::registerUpdate(SystemRegistration* sysReg)
{
    ecs->systemUpdater.registerSystem(sysReg);
}

void System::unsubscribeFromComponents(std::vector<std::type_index> unsubTypes)
{
    ecs->systemSubscriber.deregisterSystem(this, unsubTypes);
}

ComponentHandler* System::getComponentHandler(std::type_index& componentType)
{
    return this->ecs->systemSubscriber.getComponentHandler(componentType);
}
