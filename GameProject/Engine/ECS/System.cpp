#include "System.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/ECS/SystemSubscriber.hpp>

System::System(ECSCore* pECS)
    :m_pECS(pECS)
{}

System::~System()
{
    m_pECS->getSystemSubscriber()->deregisterSystem(this, componentTypes);
}

void System::subscribeToComponents(SystemRegistration* sysReg)
{
    m_pECS->getSystemSubscriber()->registerSystem(sysReg);
}

void System::registerUpdate(SystemRegistration* sysReg)
{
    m_pECS->getSystemUpdater()->registerSystem(sysReg);
}

void System::unsubscribeFromComponents(std::vector<std::type_index> unsubTypes)
{
    m_pECS->getSystemSubscriber()->deregisterSystem(this, unsubTypes);
}

ComponentHandler* System::getComponentHandler(const std::type_index& handlerType)
{
    return m_pECS->getSystemSubscriber()->getComponentHandler(handlerType);
}
