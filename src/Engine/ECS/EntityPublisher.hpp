#pragma once

#include <Engine/ECS/Entity.hpp>
#include <Engine/ECS/EntityRegistry.hpp>
#include <Engine/ECS/System.hpp>
#include <Engine/Utils/IDGenerator.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <functional>
#include <map>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class ComponentHandler;
struct ComponentHandlerRegistration;

struct EntitySubscription {
    // Stores IDs of entities found using subscription
    IDVector* subscriber;
    std::vector<std::type_index> componentTypes;
    // Optional: Called after an entity was added due to the subscription
    std::function<void(Entity)> onEntityAdded;
    // Optional: Called before an entity was removed
    std::function<void(Entity)> onEntityRemoved;
};

// Indices for subscription storage
struct SubscriptionStorageIndex {
    size_t systemID;
    // A system can have multiple subscriptions stored in an array, hence the subscription index
    size_t subIdx;
};

struct ComponentStorage {
    IDContainer* m_pContainer;
    // Optional: Used when a component needs to be destroyed by its handler. Called alongside erasing the component from its container.
    std::function<void(Entity)> m_ComponentDestructor;
};

class EntityPublisher
{
public:
    EntityPublisher(EntityRegistry* pEntityRegistry);
    ~EntityPublisher();

    void registerComponentHandler(const ComponentHandlerRegistration& componentHandlerRegistration);
    void deregisterComponentHandler(ComponentHandler* handler);
    ComponentHandler* getComponentHandler(const std::type_index& handlerType);

    // Returns a subscription ID
    size_t subscribeToComponents(const EntitySubscriberRegistration& subscriberRegistration);
    void unsubscribeFromComponents(size_t subscriptionID);

    // Notifies subscribed systems that a new component has been made
    void newComponent(Entity entityID, std::type_index componentType);
    // Notifies subscribed systems that a component has been deleted
    void removedComponent(Entity entityID, std::type_index componentType);

    std::unordered_map<std::type_index, ComponentStorage>& getComponentStorage() { return m_ComponentStorage; }

private:
    // Map component types to resources used when systems subscribe
    std::unordered_map<std::type_index, ComponentStorage> m_ComponentStorage;
    // Map component types to subscriptions. Deleted only when a subscribing system unsubscribes.
    std::unordered_multimap<std::type_index, SubscriptionStorageIndex> m_ComponentSubscriptions;

    // Map systems' IDs to their subscriptions
    IDDVector<std::vector<EntitySubscription>> m_SubscriptionStorage;
    IDGenerator m_SystemIDGenerator;

    std::unordered_map<std::type_index, ComponentHandler*> m_ComponentHandlers;

    const EntityRegistry* m_pEntityRegistry;
};
