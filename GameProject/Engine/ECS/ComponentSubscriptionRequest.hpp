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

enum SubscriptionModifier {
    REQUIRED,   // Standard option - The component is needed for the subscription
    NOT_REQUIRED    // The component is not needed for the subscription - but data accesses might be made to the component type
};

struct ComponentAccess {
    ComponentPermissions Permissions;
    std::type_index TID;
    SubscriptionModifier SubscriptionModifier = SubscriptionModifier::REQUIRED;
};

class IComponentGroup
{
public:
    virtual std::vector<ComponentAccess> toVector() const = 0;
};

// A request for a system to subscribe to one or more component types
class ComponentSubscriptionRequest {
public:
    ComponentSubscriptionRequest(std::vector<ComponentAccess> componentAccesses, std::vector<IComponentGroup*> componentGroups, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded = nullptr);
    ComponentSubscriptionRequest(std::vector<ComponentAccess> componentAccesses, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded = nullptr);
    ComponentSubscriptionRequest(std::vector<IComponentGroup*> componentGroups, IDVector* pSubscriber, std::function<void(Entity)>onEntityAdded = nullptr);

public:
    std::vector<ComponentAccess> m_ComponentAccesses;
    IDVector* m_pSubscriber;
    // Optional: Called after an entity was added due to the subscription
    std::function<void(Entity)> m_OnEntityAdded;
};
