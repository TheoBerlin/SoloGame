#include "ComponentSubscriptionRequest.hpp"

ComponentSubscriptionRequest::ComponentSubscriptionRequest(std::vector<ComponentAccess> componentAccesses, std::vector<IComponentGroup*> componentGroups, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded)
    :m_ComponentAccesses(componentAccesses),
    m_pSubscriber(pSubscriber),
    m_OnEntityAdded(onEntityAdded)
{
    for (const IComponentGroup* pComponentGroup : componentGroups) {
        const std::vector<ComponentAccess> groupAccesses = pComponentGroup->toVector();
        componentAccesses.insert(componentAccesses.end(), groupAccesses.begin(), groupAccesses.end());
    }
}

ComponentSubscriptionRequest::ComponentSubscriptionRequest(std::vector<ComponentAccess> componentAccesses, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded)
    :m_ComponentAccesses(componentAccesses),
    m_pSubscriber(pSubscriber),
    m_OnEntityAdded(onEntityAdded)
{}
