#pragma once

#include <Engine/ECS/Entity.hpp>
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
    IDVector<Entity>* subscriber;
    // Used for querying whether or not an entity has component of subscribed types
    std::vector<const IDContainer*> componentContainers;
    std::vector<std::type_index> componentTypes;
    // Optional: Called after an entity was added due to the subscription
    std::function<void(Entity)> onEntityAdded;
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

    void registerHandler(ComponentHandler* handler, const std::type_index& handlerType);
    ComponentHandler* getComponentHandler(std::type_index& handlerType);

    void registerSystem(SystemRegistration* sysReg);
    void deregisterSystem(System* system, std::vector<std::type_index>& componentTypes);

    // Notifies subscribed systems that a new component has been made
    void newComponent(Entity entityID, std::type_index componentType);
    // Notifies subscribed systems that a component has been deleted
    void removedComponent(Entity entityID, std::type_index componentType);

    void addDelayedDeletion(Entity entity);

    // Perform accumulated requests to delete entities
    void performDeletions();
    const std::vector<Entity>& getEntitiesToDelete() const;

private:
    // Map component types to resources used when systems subscribe
    std::unordered_map<std::type_index, const IDContainer*> componentContainers;
    // Map component types to subscriptions. Deleted only when a subscribing system unsubscribes.
    std::unordered_multimap<std::type_index, SubscriptionStorageIndex> componentSubscriptions;

    // Map systems' IDs to their subscriptions
    IDVector<std::vector<ComponentSubscriptions>> subscriptionStorage;
    IDGenerator systemIdGen;

    std::unordered_map<std::type_index, ComponentHandler*> componentHandlers;

    // Store all registered entity IDs and their related components. Used when deleting entities.
    IDVector<std::unordered_set<std::type_index>> registeredEntities;

    // Lists what entities to delete when performDeletions is called
    std::vector<Entity> entitiesToDelete;
};
