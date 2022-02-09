#include "EntitySubscriber.hpp"

EntitySubscriber::~EntitySubscriber()
{
    ECSCore* pECS = ECSCore::GetInstance();
    if (pECS && m_SubscriptionID != UINT32_MAX) {
        pECS->UnsubscribeFromEntities(m_SubscriptionID);
    }
}

void EntitySubscriber::SubscribeToEntities(EntitySubscriberRegistration& subscriberRegistration)
{
    for (EntitySubscriptionRegistration& subscriptionRegistration : subscriberRegistration.EntitySubscriptionRegistrations) {
        ProcessComponentGroups(subscriptionRegistration);
        ProcessExcludedTypes(subscriptionRegistration);
    }

    m_SubscriptionID = ECSCore::GetInstance()->SubscribeToEntities(subscriberRegistration);
}

void EntitySubscriber::ProcessComponentGroups(EntitySubscriptionRegistration& subscriptionRegistration)
{
    // Add the component accesses in the component groups to the component accesses vector
    const std::vector<IComponentGroup*>& componentGroups = subscriptionRegistration.ComponentGroups;
    std::vector<ComponentAccess>& componentAccesses = subscriptionRegistration.ComponentAccesses;

    for (const IComponentGroup* pComponentGroup : componentGroups) {
        const std::vector<ComponentAccess> groupAccesses = pComponentGroup->ToArray();
        componentAccesses.insert(componentAccesses.end(), groupAccesses.begin(), groupAccesses.end());
    }
}

void EntitySubscriber::ProcessExcludedTypes(EntitySubscriptionRegistration& subscriptionRegistration)
{
#ifndef CONFIG_DEBUG
    UNREFERENCED_VARIABLE(subscriptionRegistration);
#else
    const std::vector<ComponentAccess>& includedTypes = subscriptionRegistration.ComponentAccesses;
    const std::vector<const ComponentType*>& excludedTypes = subscriptionRegistration.ExcludedComponentTypes;

    for (const ComponentType* pExcludedComponent : excludedTypes) {
        for (const ComponentAccess& includedComponent : includedTypes) {
            ASSERT_MSG(pExcludedComponent != includedComponent.pTID, "The same component type was both included and excluded in an entity subscription");
        }
    }
#endif
}
