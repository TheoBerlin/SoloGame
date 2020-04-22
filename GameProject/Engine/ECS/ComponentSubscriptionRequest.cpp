#include "ComponentSubscriptionRequest.hpp"

ComponentSubscriptionRequest::ComponentSubscriptionRequest(std::vector<ComponentAccess> componentAccesses, std::vector<IComponentGroup*> componentGroups, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded)
    :m_pSubscriber(pSubscriber),
    m_OnEntityAdded(onEntityAdded)
{
    // Add the component accesses in the component groups to the component accesses vector
    for (const IComponentGroup* pComponentGroup : componentGroups) {
        const std::vector<ComponentAccess> groupAccesses = pComponentGroup->toVector();
        componentAccesses.insert(componentAccesses.end(), groupAccesses.begin(), groupAccesses.end());
    }

    m_ComponentAccesses = componentAccesses;
}

ComponentSubscriptionRequest::ComponentSubscriptionRequest(std::vector<ComponentAccess> componentAccesses, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded)
    :ComponentSubscriptionRequest(componentAccesses, {}, pSubscriber, onEntityAdded)
{}

ComponentSubscriptionRequest::ComponentSubscriptionRequest(std::vector<IComponentGroup*> componentGroups, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded)
    :ComponentSubscriptionRequest({}, componentGroups, pSubscriber, onEntityAdded)
{}
