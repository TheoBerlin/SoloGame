#include "System.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/ECS/ComponentSubscriber.hpp>

System::System(ECSCore* pECS)
    :m_pECS(pECS),
    m_ComponentSubscriptionID(UINT64_MAX),
    m_SystemID(UINT64_MAX)
{}

System::~System()
{
    m_pECS->getComponentSubscriber()->unsubscribeFromComponents(m_ComponentSubscriptionID);
}

void System::enqueueRegistration(const SystemRegistration& sysReg)
{
    m_pECS->enqueueSystemRegistration(sysReg);
}

ComponentHandler* System::getComponentHandler(const std::type_index& handlerType)
{
    return m_pECS->getComponentSubscriber()->getComponentHandler(handlerType);
}
