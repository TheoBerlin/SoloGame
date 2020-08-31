#include "EntitySubscriber.hpp"

#include <Engine/ECS/ECSCore.hpp>

EntitySubscriptionRegistration::EntitySubscriptionRegistration(std::vector<ComponentAccess> componentAccesses, const std::vector<IComponentGroup*>& componentGroups, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded, std::function<void(Entity)>onEntityRemoved)
    :m_pSubscriber(pSubscriber),
    m_OnEntityAdded(onEntityAdded),
    m_OnEntityRemoved(onEntityRemoved)
{
    // Add the component accesses in the component groups to the component accesses vector
    for (const IComponentGroup* pComponentGroup : componentGroups) {
        const std::vector<ComponentAccess> groupAccesses = pComponentGroup->toVector();
        componentAccesses.insert(componentAccesses.end(), groupAccesses.begin(), groupAccesses.end());
    }

    m_ComponentAccesses = componentAccesses;
}

EntitySubscriptionRegistration::EntitySubscriptionRegistration(const std::vector<ComponentAccess>& componentAccesses, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded, std::function<void(Entity)>onEntityRemoved)
    :EntitySubscriptionRegistration(componentAccesses, {}, pSubscriber, onEntityAdded, onEntityRemoved)
{}

EntitySubscriptionRegistration::EntitySubscriptionRegistration(const std::vector<IComponentGroup*>& componentGroups, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded, std::function<void(Entity)>onEntityRemoved)
    :EntitySubscriptionRegistration({}, componentGroups, pSubscriber, onEntityAdded, onEntityRemoved)
{}

EntitySubscriber::EntitySubscriber(ECSCore* pECS)
    :m_pECS(pECS),
    m_SubscriptionID(UINT64_MAX)
{}

EntitySubscriber::~EntitySubscriber()
{
    m_pECS->getEntityPublisher()->unsubscribeFromComponents(m_SubscriptionID);
}

void EntitySubscriber::subscribeToEntities(const EntitySubscriberRegistration& subscriberRegistration, const std::function<bool()>& initFn)
{
    m_pECS->enqueueEntitySubscriptions(subscriberRegistration, initFn, &m_SubscriptionID);
}
