#pragma once

#include <Engine/ECS/Entity.hpp>

#include <functional>
#include <typeindex>
#include <vector>

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

// A request for a system to subscribe to one or more component types
class ComponentSubscriptionRequest {
public:
    ComponentSubscriptionRequest(std::vector<ComponentAccess> componentAccesses, std::vector<IComponentGroup*> componentGroups, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded = nullptr, std::function<void(Entity)>onEntityRemoved = nullptr);
    ComponentSubscriptionRequest(std::vector<ComponentAccess> componentAccesses, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded = nullptr, std::function<void(Entity)>onEntityRemoved = nullptr);
    ComponentSubscriptionRequest(std::vector<IComponentGroup*> componentGroups, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded = nullptr, std::function<void(Entity)>onEntityRemoved = nullptr);

public:
    std::vector<ComponentAccess> m_ComponentAccesses;
    IDVector* m_pSubscriber;
    // Optional: Called after an entity was added due to the subscription
    std::function<void(Entity)> m_OnEntityAdded;
    // Optional: Called before an entity was removed
    std::function<void(Entity)> m_OnEntityRemoved;
};

struct ComponentSubscriberRegistration {
    std::vector<ComponentSubscriptionRequest> ComponentSubscriptionRequests;
    std::vector<ComponentAccess> AdditionalDependencies;
};
