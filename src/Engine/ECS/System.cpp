#include "System.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/ECS/ComponentPublisher.hpp>

System::System(ECSCore* pECS)
    :EntitySubscriber(pECS),
    m_pECS(pECS),
    m_SystemID(UINT64_MAX)
{}

void System::enqueueRegistration(const SystemRegistration& sysReg)
{
    std::function<bool()> initFunction = [this, sysReg] {
        if (!initSystem()) {
            return false;
        }

        m_pECS->getSystemRegistry()->registerSystem(sysReg);
        return true;
    };

    subscribeToEntities(sysReg.SubscriberRegistration, initFunction);
}

ComponentHandler* System::getComponentHandler(const std::type_index& handlerType)
{
    return m_pECS->getComponentPublisher()->getComponentHandler(handlerType);
}
