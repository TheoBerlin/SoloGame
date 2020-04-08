#include "System.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/ECS/SystemSubscriber.hpp>

System::System(ECSCore* pECS)
    :m_pECS(pECS)
{}

System::~System()
{
    m_pECS->getSystemSubscriber()->unsubscribeFromComponents(m_ComponentSubscriptionID, m_ComponentSubscriptionTypes);
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
    return m_pECS->getSystemSubscriber()->getComponentHandler(handlerType);
}
