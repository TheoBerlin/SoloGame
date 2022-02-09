#pragma once

#include "Engine/ECS/Entity.hpp"
#include "Engine/ECS/EntityRegistry.hpp"
#include "Engine/ECS/System.hpp"
#include "Engine/Utils/IDVector.hpp"
#include "Engine/Utils/IDGenerator.hpp"

#include <functional>
#include <map>
#include <typeindex>
#include <unordered_set>

class ComponentStorage;

struct EntitySubscription {
    // Stores IDs of entities found using subscription
    IDVector* pSubscriber;
    std::vector<const ComponentType*> ComponentTypes;
    std::vector<const ComponentType*> ExcludedComponentTypes;
    // Optional: Called after an entity was added due to the subscription
    std::function<void(Entity)> OnEntityAdded;
    // Optional: Called before an entity was removed
    std::function<void(Entity)> OnEntityRemoval;
};

// Indices for subscription storage
struct SubscriptionStorageIndex {
    uint32_t SystemID;
    // A system can have multiple subscriptions stored in an array, hence the subscription index
    uint32_t SubIdx;
};

class EntityPublisher
{
public:
    EntityPublisher(const ComponentStorage* pComponentStorage, const EntityRegistry* pEntityRegistry);
    ~EntityPublisher() = default;

    // Returns a subscription ID
    uint32_t SubscribeToEntities(const EntitySubscriberRegistration& subscriberRegistration);
    void UnsubscribeFromEntities(uint32_t subscriptionID);

    // Notifies subscribers that a component has been added
    void PublishComponent(Entity entity, const ComponentType* pComponentType);
    // Notifies subscribers that a component has been deleted
    void UnpublishComponent(Entity entity, const ComponentType* pComponentType);

private:
    static void EliminateDuplicateTIDs(std::vector<const ComponentType*>& TIDs);

private:
    // Map component types to subscriptions. Deleted only when a subscribing system unsubscribes.
    std::unordered_multimap<const ComponentType*, SubscriptionStorageIndex> m_ComponentSubscriptions;

    // Map systems' IDs to their subscriptions
    IDDVector<std::vector<EntitySubscription>> m_SubscriptionStorage;
    IDGenerator m_SystemIDGenerator;

    const ComponentStorage* m_pComponentStorage;
    const EntityRegistry* m_pEntityRegistry;
};
