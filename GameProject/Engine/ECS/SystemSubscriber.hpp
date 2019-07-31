#pragma once

#include <Engine/ECS/Entity.hpp>
#include <Engine/ECS/System.hpp>
#include <Engine/Utils/IDGenerator.hpp>
#include <Engine/Utils/IDVector.hpp>

#include <functional>
#include <map>
#include <typeindex>
#include <unordered_map>
#include <vector>

class ComponentHandler;
struct ComponentRegistration;

struct ComponentSubscriptions {
    // Pointer to a system's entity ID vector
    IDVector<Entity>* subscriber;
    // Functions for querying whether or not an entity has component of subscribed types
    std::vector<std::function<bool(Entity)>*> entityHasComponent;

    std::vector<std::type_index> componentTypes;
};

struct ComponentResources {
    std::function<bool(Entity)> componentQuery;
    const std::vector<size_t>* entities;
};

struct SubscriptionStorageIndex {
    size_t systemID;
    size_t subIdx;
};

class SystemSubscriber
{
public:
    SystemSubscriber();
    ~SystemSubscriber();

    // Associates a component types with functions for querying for their existence given entity IDs
    void registerComponents(std::vector<ComponentRegistration>* componentRegs);
    void deregisterComponents(ComponentHandler* handler);

    void registerHandler(ComponentHandler* handler, std::type_index& handlerType);
    ComponentHandler* getComponentHandler(std::type_index& handlerType);

    void registerSystem(SystemRegistration* sysReg);
    void deregisterSystem(System* system, std::vector<std::type_index>& componentTypes);

    // Notifies subscribed systems that a new component has been made
    void newComponent(Entity entityID, std::type_index componentType);
    // Notifies subscribed systems that a component has been deleted
    void removedComponent(Entity entityID, std::type_index componentType);

private:
    // Map component types to resources used when systems subscribe
    std::unordered_map<std::type_index, ComponentResources> componentResources;
    // Map component types to subscriptions. Deleted only when a subscribing system unsubscribes.
    std::unordered_multimap<std::type_index, SubscriptionStorageIndex> componentSubscriptions;

    // Map systems' IDs to their subscriptions
    IDVector<std::vector<ComponentSubscriptions>> subscriptionStorage;
    IDGenerator systemIdGen;

    std::unordered_map<std::type_index, ComponentHandler*> componentHandlers;
};
