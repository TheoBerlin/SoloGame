#include "System.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/ECS/ComponentSubscriber.hpp>

System::System(ECSCore* pECS)
    :m_pECS(pECS)
{}

System::~System()
{
    m_pECS->getComponentSubscriber()->unsubscribeFromComponents(m_ComponentSubscriptionID);
}

void System::subscribeToComponents(const SystemRegistration& sysReg)
{
    m_pECS->enqueueSystemRegistration(sysReg);
}

void System::registerUpdate(const SystemRegistration& sysReg)
{
    m_pECS->getSystemUpdater()->registerSystem(sysReg);
}

ComponentHandler* System::getComponentHandler(const std::type_index& handlerType)
{
    return m_pECS->getComponentSubscriber()->getComponentHandler(handlerType);
}
