#include "ComponentSubscriptionRequest.hpp"

ComponentSubscriptionRequest::ComponentSubscriptionRequest(std::vector<ComponentAccess> componentAccesses, std::vector<IComponentGroup*> componentGroups, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded, std::function<void(Entity)>onEntityRemoved)
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

ComponentSubscriptionRequest::ComponentSubscriptionRequest(std::vector<ComponentAccess> componentAccesses, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded, std::function<void(Entity)>onEntityRemoved)
    :ComponentSubscriptionRequest(componentAccesses, {}, pSubscriber, onEntityAdded, onEntityRemoved)
{}

ComponentSubscriptionRequest::ComponentSubscriptionRequest(std::vector<IComponentGroup*> componentGroups, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded, std::function<void(Entity)>onEntityRemoved)
    :ComponentSubscriptionRequest({}, componentGroups, pSubscriber, onEntityAdded, onEntityRemoved)
{}
