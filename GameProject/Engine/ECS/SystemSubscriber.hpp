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
class IComponentContainer;
struct ComponentRegistration;

struct ComponentSubscriptions {
    // Stores IDs of entities found using subscription
    IDVector<Entity>* subscriber; // TODO: Rename to... "foundEntities"?
    std::vector<std::type_index> componentTypes;
    // Optional: Called after an entity was added due to the subscription
    std::function<void(Entity)> onEntityAdded;
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

class SystemSubscriber
{
public:
    SystemSubscriber(EntityRegistry* pEntityRegistry);
    ~SystemSubscriber();

    // Associates a component types with functions for querying for their existence given entity IDs
    void registerComponentHandler(std::vector<ComponentRegistration>* componentRegs);
    void deregisterComponentHandler(ComponentHandler* handler);

    void registerHandler(ComponentHandler* handler, const std::type_index& handlerType);
    ComponentHandler* getComponentHandler(const std::type_index& handlerType);

    void registerSystem(SystemRegistration* sysReg);
    void deregisterSystem(System* system, std::vector<std::type_index>& componentTypes);

    // Notifies subscribed systems that a new component has been made
    void newComponent(Entity entityID, std::type_index componentType);
    // Notifies subscribed systems that a component has been deleted
    void removedComponent(Entity entityID, std::type_index componentType);

    std::unordered_map<std::type_index, ComponentStorage>& getComponentStorage() { return m_ComponentStorage; }

private:
    // Map component types to resources used when systems subscribe
    std::unordered_map<std::type_index, ComponentStorage> m_ComponentStorage;
    // Map component types to subscriptions. Deleted only when a subscribing system unsubscribes.
    std::unordered_multimap<std::type_index, SubscriptionStorageIndex> componentSubscriptions;

    // Map systems' IDs to their subscriptions
    IDVector<std::vector<ComponentSubscriptions>> subscriptionStorage;
    IDGenerator systemIdGen;

    std::unordered_map<std::type_index, ComponentHandler*> componentHandlers;

    const EntityRegistry* m_pEntityRegistry;
};
