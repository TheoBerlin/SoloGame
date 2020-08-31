#include "System.hpp"

#include <Engine/ECS/ECSCore.hpp>
#include <Engine/ECS/EntityPublisher.hpp>

System::System(ECSCore* pECS)
    :EntitySubscriber(pECS),
    RegularWorker(pECS),
    m_pECS(pECS),
    m_SystemID(UINT64_MAX)
{}

void System::enqueueRegistration(const SystemRegistration& systemRegistration)
{
    Job job = {
        .Function = [this] {
            update(m_pECS->getDeltaTime());
        },
        .Components = getUniqueComponentAccesses(systemRegistration.SubscriberRegistration)
    };

    uint32_t phase = systemRegistration.Phase;
    std::function<bool()> initFunction = [this, phase, job] {
        if (!initSystem()) {
            return false;
        }

        m_pECS->getDeltaTime();

        scheduleRegularWork(job, phase);
        return true;
    };

    subscribeToEntities(systemRegistration.SubscriberRegistration, initFunction);
}

ComponentHandler* System::getComponentHandler(const std::type_index& handlerType)
{
    return m_pECS->getEntityPublisher()->getComponentHandler(handlerType);
}
