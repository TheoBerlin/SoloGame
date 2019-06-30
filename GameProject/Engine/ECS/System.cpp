#include "System.hpp"

#include <Engine/ECS/SystemSubscriber.hpp>

System::System(SystemSubscriber* systemSubscriber)
    :systemSubscriber(systemSubscriber)
{}

System::~System()
{}

void System::subscribeToComponents(std::vector<ComponentSubReq>* subReqs)
{
    systemSubscriber->registerSystem(this, subReqs);
}

void System::unsubscribeFromComponents(std::vector<std::type_index> unsubTypes)
{
    systemSubscriber->deregisterSystem(this, unsubTypes);
}

ComponentHandler* System::getComponentHandler(std::type_index& componentType)
{
    return this->systemSubscriber->getComponentHandler(componentType);
}
