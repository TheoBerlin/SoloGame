#pragma once

#include <Engine/ECS/Entity.hpp>

#include <functional>
#include <typeindex>
#include <vector>

class ECSCore;
class IDVector;

enum ComponentPermissions {
    NDA = 0, // No Data Access
    R   = 1,
    RW  = 2
};

struct ComponentAccess {
    ComponentPermissions Permissions;
    std::type_index TID;
};

class IComponentGroup
{
public:
    virtual std::vector<ComponentAccess> toVector() const = 0;
};

// EntitySubscriptionRegistration contains all required information to request a single entity subscription
class EntitySubscriptionRegistration {
public:
    EntitySubscriptionRegistration(std::vector<ComponentAccess> componentAccesses, const std::vector<IComponentGroup*>& componentGroups, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded = nullptr, std::function<void(Entity)>onEntityRemoved = nullptr);
    EntitySubscriptionRegistration(const std::vector<ComponentAccess>& componentAccesses, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded = nullptr, std::function<void(Entity)>onEntityRemoved = nullptr);
    EntitySubscriptionRegistration(const std::vector<IComponentGroup*>& componentGroups, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded = nullptr, std::function<void(Entity)>onEntityRemoved = nullptr);

public:
    std::vector<ComponentAccess> m_ComponentAccesses;
    IDVector* m_pSubscriber;
    // Optional: Called after an entity was added due to the subscription
    std::function<void(Entity)> m_OnEntityAdded;
    // Optional: Called before an entity was removed
    std::function<void(Entity)> m_OnEntityRemoved;
};

// EntitySubscriberRegistration is a complete set of data required to register a component subscriber
struct EntitySubscriberRegistration {
    std::vector<EntitySubscriptionRegistration> EntitySubscriptionRegistrations;
    /*  AdditionalDependencies are components that the subscriber will process.
        However, the subscriber will not store an array of the entities whose components it will process. */
    std::vector<ComponentAccess> AdditionalDependencies;
};

// EntitySubscriber deregisters its entity subscriptions at destruction
class EntitySubscriber
{
public:
    EntitySubscriber(ECSCore* pECS);
    ~EntitySubscriber();

    // subscribeToEntities enqueues entity subscriptions. initFn is called when all dependencies have been initialized.
    void subscribeToEntities(const EntitySubscriberRegistration& subscriberRegistration, const std::function<bool()>& initFn);

private:
    ECSCore* m_pECS;
    size_t m_SubscriptionID;
};
