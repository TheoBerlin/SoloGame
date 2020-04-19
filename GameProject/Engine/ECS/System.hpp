#pragma once

#include <Engine/ECS/Entity.hpp>
#include <Engine/Utils/IDVector.hpp>
#include <functional>
#include <typeindex>
#include <vector>

enum ComponentPermissions {
    R = 0,
    RW = 1
};

struct ComponentUpdateReg {
    ComponentPermissions permissions;
    std::type_index tid;
};

// A request for a system to subscribe to one or more component types
struct ComponentSubscriptionRequest {
    std::vector<ComponentUpdateReg> componentTypes;
    IDVector* subscriber;
    // Optional: Called after an entity was added due to the subscription
    std::function<void(Entity)> onEntityAdded;
};

class System;

struct SystemRegistration {
    std::vector<ComponentSubscriptionRequest> SubscriptionRequests;
    System* pSystem;
};

class ComponentHandler;
class ECSCore;
class ComponentSubscriber;
struct ComponentSubscriptionRequest;

/*
    A system stores pointers to component storages and processes the components each frame
    in the update function
*/
class System
{
public:
    // Registers the system in the system handler
    System(ECSCore* pECS);

    // Deregisters system
    ~System();

    virtual bool init() = 0;

    virtual void update(float dt) = 0;

    inline size_t getSystemID() const { return m_SystemID; }
    void setSystemID(size_t systemID) { m_SystemID = systemID; }

    void setComponentSubscriptionID(size_t ID) { m_ComponentSubscriptionID = ID; }

protected:
    void subscribeToComponents(const SystemRegistration& sysReg);
    void registerUpdate(const SystemRegistration& sysReg);

    ComponentHandler* getComponentHandler(const std::type_index& handlerType);

private:
    ECSCore* m_pECS;

    size_t m_ComponentSubscriptionID;
    size_t m_SystemID;
};
